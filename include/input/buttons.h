#ifndef INPUT_BUTTONS_H
#define INPUT_BUTTONS_H

#include <stdint.h>

typedef enum {
    BTN_EVT_NONE = 0,
    BTN_EVT_NEXT,
    BTN_EVT_PREV
} ButtonEvent;

typedef struct Buttons Buttons;

/**
 * Buttons init
 * @param out  oluşturulan Buttons*
 * @return 0 başarı, -1 hata
 */
int buttons_init(Buttons** out);

/**
 * Buttons deinit
 */
void buttons_deinit(Buttons* b);

/**
 * Poll buttons with debounce
 * @param b       Buttons*
 * @param now_ms  monotonic time in ms
 * @return ButtonEvent
 */
ButtonEvent buttons_poll(Buttons* b, uint64_t now_ms);

#endif
