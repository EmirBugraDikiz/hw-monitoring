#define _DEFAULT_SOURCE

#include "input/buttons.h"
#include "pins.h"

#include <gpiod.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =======================
 * Internal structures
 * ======================= */

typedef struct {
    unsigned offset;
    int stable;          // 0 = not pressed, 1 = pressed
    int last_stable;
    int raw_last;
    uint64_t last_change_ms;
} DebouncedPin;

struct Buttons {
    struct gpiod_chip* chip;
    struct gpiod_line_request* req;

    DebouncedPin next;
    DebouncedPin prev;

    uint64_t debounce_ms;
};

/* =======================
 * Helpers
 * ======================= */

/*
 * libgpiod v2 returns:
 *   0 -> inactive
 *   1 -> active
 *
 * We NORMALIZE:
 *   0 -> not pressed
 *   1 -> pressed
 */
static int read_req(struct gpiod_line_request* req, unsigned offset)
{
    int v = gpiod_line_request_get_value(req, offset);
    if (v < 0) return v;
    return (v != 0) ? 1 : 0;
}

static ButtonEvent update_pin(DebouncedPin* p,
                              int raw,
                              uint64_t now_ms,
                              uint64_t debounce_ms,
                              ButtonEvent evt_on_press)
{
    /* raw level changed */
    if (raw != p->raw_last) {
        p->raw_last = raw;
        p->last_change_ms = now_ms;
    }

    /* debounce window passed */
    if ((now_ms - p->last_change_ms) >= debounce_ms) {
        p->last_stable = p->stable;
        p->stable = p->raw_last;

        /* rising edge: 0 -> 1 = button press */
        if (p->last_stable == 0 && p->stable == 1) {
            return evt_on_press;
        }
    }

    return BTN_EVT_NONE;
}

/* =======================
 * Public API
 * ======================= */

int buttons_init(Buttons** out)
{
    if (!out) return -1;

    Buttons* b = calloc(1, sizeof(*b));
    if (!b) return -1;

    b->debounce_ms = 50;   // default debounce

    b->chip = gpiod_chip_open(GPIO_CHIP_PATH);
    if (!b->chip) {
        free(b);
        return -1;
    }

    struct gpiod_request_config* rconf = gpiod_request_config_new();
    struct gpiod_line_settings* lset  = gpiod_line_settings_new();
    struct gpiod_line_config* lconf   = gpiod_line_config_new();

    if (!rconf || !lset || !lconf) {
        buttons_deinit(b);
        return -1;
    }

    gpiod_request_config_set_consumer(rconf, "hwmon_buttons");
    gpiod_line_settings_set_direction(lset, GPIOD_LINE_DIRECTION_INPUT);

    /* External pull-up kullanıyoruz */
    unsigned offsets[2] = { PIN_BTN_NEXT, PIN_BTN_PREV };

    if (gpiod_line_config_add_line_settings(lconf, offsets, 2, lset) != 0) {
        gpiod_request_config_free(rconf);
        gpiod_line_settings_free(lset);
        gpiod_line_config_free(lconf);
        buttons_deinit(b);
        return -1;
    }

    b->req = gpiod_chip_request_lines(b->chip, rconf, lconf);

    gpiod_request_config_free(rconf);
    gpiod_line_settings_free(lset);
    gpiod_line_config_free(lconf);

    if (!b->req) {
        buttons_deinit(b);
        return -1;
    }

    b->next.offset = PIN_BTN_NEXT;
    b->prev.offset = PIN_BTN_PREV;

    int rnext = read_req(b->req, b->next.offset);
    int rprev = read_req(b->req, b->prev.offset);

    /* default: not pressed */
    b->next.raw_last =
    b->next.stable =
    b->next.last_stable = (rnext < 0 ? 0 : rnext);

    b->prev.raw_last =
    b->prev.stable =
    b->prev.last_stable = (rprev < 0 ? 0 : rprev);

    *out = b;
    return 0;
}

void buttons_deinit(Buttons* b)
{
    if (!b) return;
    if (b->req)  gpiod_line_request_release(b->req);
    if (b->chip) gpiod_chip_close(b->chip);
    free(b);
}

ButtonEvent buttons_poll(Buttons* b, uint64_t now_ms)
{
    if (!b || !b->req) return BTN_EVT_NONE;

    int raw_next = read_req(b->req, b->next.offset);
    int raw_prev = read_req(b->req, b->prev.offset);

    if (raw_next < 0 || raw_prev < 0)
        return BTN_EVT_NONE;

    ButtonEvent e;

    e = update_pin(&b->next, raw_next, now_ms,
                   b->debounce_ms, BTN_EVT_NEXT);
    if (e != BTN_EVT_NONE) return e;

    e = update_pin(&b->prev, raw_prev, now_ms,
                   b->debounce_ms, BTN_EVT_PREV);
    return e;
}
