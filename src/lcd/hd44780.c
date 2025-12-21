#define _DEFAULT_SOURCE
#include "lcd/hd44780.h"
#include "pins.h"

#include <gpiod.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Hd44780 {
    struct gpiod_chip* chip;
    struct gpiod_line_request* req;

    unsigned rs, e, d4, d5, d6, d7;
};

static void sleep_us(long us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000L;
    ts.tv_nsec = (us % 1000000L) * 1000L;
    nanosleep(&ts, NULL);
}

static int setv(Hd44780* lcd, unsigned offset, int v) {
    return gpiod_line_request_set_value(lcd->req, offset, v); // v2 API
}

static int write4(Hd44780* lcd, int nibble) {
    // nibble b3..b0 -> D7..D4
    if (setv(lcd, lcd->d4, (nibble >> 0) & 1) != 0) return -1;
    if (setv(lcd, lcd->d5, (nibble >> 1) & 1) != 0) return -1;
    if (setv(lcd, lcd->d6, (nibble >> 2) & 1) != 0) return -1;
    if (setv(lcd, lcd->d7, (nibble >> 3) & 1) != 0) return -1;

    // E pulse
    if (setv(lcd, lcd->e, 1) != 0) return -1;
    sleep_us(1);
    if (setv(lcd, lcd->e, 0) != 0) return -1;

    // komut/data yazım süresi
    sleep_us(40);
    return 0;
}

static int send_byte(Hd44780* lcd, int rs, int byte) {
    if (setv(lcd, lcd->rs, rs) != 0) return -1;
    if (write4(lcd, (byte >> 4) & 0x0F) != 0) return -1;
    if (write4(lcd, (byte >> 0) & 0x0F) != 0) return -1;
    return 0;
}

static int cmd(Hd44780* lcd, int c) { return send_byte(lcd, 0, c); }
static int dat(Hd44780* lcd, int d) { return send_byte(lcd, 1, d); }

int hd44780_init(Hd44780** out) {
    if (!out) return -1;

    Hd44780* lcd = calloc(1, sizeof(*lcd));
    if (!lcd) return -1;

    lcd->rs = PIN_LCD_RS;
    lcd->e  = PIN_LCD_E;
    lcd->d4 = PIN_LCD_D4;
    lcd->d5 = PIN_LCD_D5;
    lcd->d6 = PIN_LCD_D6;
    lcd->d7 = PIN_LCD_D7;

    lcd->chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if (!lcd->chip) { free(lcd); return -1; }

    struct gpiod_request_config* rconf = gpiod_request_config_new();
    struct gpiod_line_settings* lset  = gpiod_line_settings_new();
    struct gpiod_line_config* lconf   = gpiod_line_config_new();
    if (!rconf || !lset || !lconf) { hd44780_deinit(lcd); return -1; }

    gpiod_request_config_set_consumer(rconf, "hd44780");
    gpiod_line_settings_set_direction(lset, GPIOD_LINE_DIRECTION_OUTPUT);

    unsigned offsets[6] = { lcd->rs, lcd->e, lcd->d4, lcd->d5, lcd->d6, lcd->d7 };

    if (gpiod_line_config_add_line_settings(lconf, offsets, 6, lset) != 0) {
        gpiod_request_config_free(rconf);
        gpiod_line_settings_free(lset);
        gpiod_line_config_free(lconf);
        hd44780_deinit(lcd);
        return -1;
    }

    lcd->req = gpiod_chip_request_lines(lcd->chip, rconf, lconf);

    gpiod_request_config_free(rconf);
    gpiod_line_settings_free(lset);
    gpiod_line_config_free(lconf);

    if (!lcd->req) { hd44780_deinit(lcd); return -1; }

    // initial values low
    for (int i = 0; i < 6; i++) gpiod_line_request_set_value(lcd->req, offsets[i], 0);

    // power-on wait
    sleep_us(50000);

    // 4-bit init sequence
    setv(lcd, lcd->rs, 0);
    write4(lcd, 0x03); sleep_us(5000);
    write4(lcd, 0x03); sleep_us(200);
    write4(lcd, 0x03); sleep_us(200);
    write4(lcd, 0x02); sleep_us(200);

    cmd(lcd, 0x28); // 4-bit, 2 line, 5x8
    cmd(lcd, 0x08); // display off
    cmd(lcd, 0x01); // clear
    sleep_us(2000);
    cmd(lcd, 0x06); // entry mode
    cmd(lcd, 0x0C); // display on, cursor off

    *out = lcd;
    return 0;
}

void hd44780_deinit(Hd44780* lcd) {
    if (!lcd) return;
    if (lcd->req) gpiod_line_request_release(lcd->req);
    if (lcd->chip) gpiod_chip_close(lcd->chip);
    free(lcd);
}

int hd44780_clear(Hd44780* lcd) {
    if (!lcd) return -1;
    if (cmd(lcd, 0x01) != 0) return -1;
    sleep_us(2000);
    return 0;
}

int hd44780_set_cursor(Hd44780* lcd, int row, int col) {
    if (!lcd) return -1;
    if (col < 0) col = 0;
    if (col > 15) col = 15;
    int addr = (row == 0) ? (0x00 + col) : (0x40 + col);
    return cmd(lcd, 0x80 | addr);
}

int hd44780_write_str(Hd44780* lcd, const char* s) {
    if (!lcd || !s) return -1;
    for (; *s; s++) {
        if (dat(lcd, (unsigned char)*s) != 0) return -1;
    }
    return 0;
}

static void write_padded_line(Hd44780* lcd, int row, const char line[17]) {
    hd44780_set_cursor(lcd, row, 0);
    for (int i = 0; i < 16; i++) {
        char c = line[i] ? line[i] : ' ';
        dat(lcd, (unsigned char)c);
    }
}

int hd44780_write_lines(Hd44780* lcd, const char line1[17], const char line2[17]) {
    if (!lcd) return -1;
    write_padded_line(lcd, 0, line1);
    write_padded_line(lcd, 1, line2);
    return 0;
}
