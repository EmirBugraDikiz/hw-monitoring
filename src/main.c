#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#include "hardware_stats.h"
#include "page_manager.h"
#include "lcd/hd44780.h"
#include "input/buttons.h"

static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int sig) {
    (void)sig;
    g_stop = 1;
}

static uint64_t now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

int main(void) {
    signal(SIGINT, on_sigint);
    signal(SIGTERM, on_sigint);

    PageManager pm;
    if (page_manager_init(&pm) != 0) {
        fprintf(stderr, "page_manager_init failed\n");
        return 1;
    }

    Hd44780* lcd = NULL;
    if (hd44780_init(&lcd) != 0) {
        fprintf(stderr, "hd44780_init failed (wiring/pins?)\n");
        return 1;
    }
    hd44780_clear(lcd);

    Buttons* btn = NULL;
    if (buttons_init(&btn) != 0) {
        fprintf(stderr, "buttons_init failed\n");
        btn = NULL; // LCD yine de çalışsın
    }

    HardwareStats s;
    uint64_t last_stats_ms = 0;

    while (!g_stop) {
        uint64_t t = now_ms();

        // 1 saniyede bir stats oku (CPU % doğru olsun diye daha mantıklı)
        if (t - last_stats_ms >= 1000) {
            if (read_system_stats(&s) != 0) {
                hd44780_write_lines(lcd, "read_system_stats", "failed           ");
            } else {
                last_stats_ms = t;
            }
        }

        // buton event (20ms polling yeter)
        if (btn) {
            ButtonEvent e = buttons_poll(btn, t);
            if (e == BTN_EVT_NEXT) page_manager_next(&pm);
            else if (e == BTN_EVT_PREV) page_manager_prev(&pm);
        }

        char l1[17], l2[17];
        page_manager_render(&pm, &s, l1, l2);
        hd44780_write_lines(lcd, l1, l2);

        usleep(20000); // 20ms
    }

    // çıkışta lcd temizle
    hd44780_clear(lcd);

    if (btn) buttons_deinit(btn);
    hd44780_deinit(lcd);
    page_manager_deinit(&pm);
    return 0;
}
