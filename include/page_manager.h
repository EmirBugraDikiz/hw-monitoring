#ifndef PAGE_MANAGER_H
#define PAGE_MANAGER_H

#include <stddef.h>
#include "page.h"
#include "hardware_stats.h"

#ifdef _cplusplus
extern "C" {
#endif

typedef struct {

    Page* head;
    Page* current;
    size_t count;

}PageManager;


int page_manager_init(PageManager* pm);    

void page_manager_deinit(PageManager* pm);

void page_manager_next(PageManager* pm);

void page_manager_prev(PageManager* pm);

void page_manager_render(const PageManager* pm, const HardwareStats* stats, char line1[LCD_COLS + 1], char line2[LCD_COLS + 1]);

const char* page_manager_current_name(const PageManager* pm);

#ifdef _cplusplus
}
#endif


#endif