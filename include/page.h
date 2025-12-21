#ifndef PAGE_H
#define PAGE_H

#define LCD_COLS 16
#include "hardware_stats.h"



typedef struct Page Page;


typedef void(*page_render_fn)(const Page* page, const HardwareStats* stats, char line1[LCD_COLS + 1], char line2[LCD_COLS + 1]);


struct Page {

    const char* name;
    page_render_fn render;
    Page* next;
    Page* prev;

};


#endif