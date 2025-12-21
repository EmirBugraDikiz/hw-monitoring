#ifndef HD44780_H
#define HD44780_H

typedef struct Hd44780 Hd44780;

int  hd44780_init(Hd44780** out);
void hd44780_deinit(Hd44780* lcd);

int  hd44780_clear(Hd44780* lcd);
int  hd44780_set_cursor(Hd44780* lcd, int row, int col);
int  hd44780_write_str(Hd44780* lcd, const char* s);

// 16x2 tek çağrı (flicker azaltır)
int  hd44780_write_lines(Hd44780* lcd, const char line1[17], const char line2[17]);

#endif