#ifndef PINS_H
#define PINS_H

// LCD pins (BCM GPIO numbers)
#define PIN_LCD_RS  26
#define PIN_LCD_E   19
#define PIN_LCD_D4  13
#define PIN_LCD_D5   6
#define PIN_LCD_D6   5
#define PIN_LCD_D7  11

// Buttons (BCM GPIO numbers)
#define PIN_BTN_NEXT 20
#define PIN_BTN_PREV 21

// libgpiod chip device
#define GPIO_CHIP_PATH "/dev/gpiochip0"


#endif