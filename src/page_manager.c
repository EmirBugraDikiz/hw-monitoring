#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "page_manager.h"
#include "page.h"
#include "hardware_stats.h"


static void pad16(char line[LCD_COLS + 1]){

    size_t n = strlen(line);
    
    if(n > LCD_COLS){

        line[LCD_COLS] = '\0';
        return;

    }

    for(size_t i = n; i < LCD_COLS; i++) line[i] = ' ';
    line[LCD_COLS] = '\0';


}



static void render_cpu_page(const Page* page, const HardwareStats* s, char line1[LCD_COLS + 1], char line2[LCD_COLS + 1]){

    (void)page;

    if(s->cpu_temp_c > 0.0){

        snprintf(line1, LCD_COLS + 1, "CPU:%5.1f%% %2.0fC", s->cpu_usage_percent, s->cpu_temp_c);

    }


    else{

        snprintf(line1, LCD_COLS + 1, "CPU:%5.1f%%", s->cpu_usage_percent);

    }


    snprintf(line2, LCD_COLS + 1, "Load:%5.2f", s->load1);

    pad16(line1);
    pad16(line2);


}


static void render_ram_page(const Page* page, const HardwareStats* s, char line1[LCD_COLS + 1], char line2[LCD_COLS + 1]){

    (void)page;


    double used_mb = (s->mem_total_kb - s->mem_available_kb) / 1024.0;
    double total_mb = (s->mem_total_kb) / 1024.0;

    snprintf(line1, LCD_COLS + 1, "RAM:%5.0f/%4.0f", used_mb, total_mb);

    double used_mem_percent = 0.0;
    if(s->mem_total_kb > 0){

        used_mem_percent = 100.0 * (double)(s->mem_total_kb - s->mem_available_kb) / (double)(s->mem_total_kb);
    }

    snprintf(line2, LCD_COLS + 1, "Used:%6.1f%%", used_mem_percent);


    pad16(line1);
    pad16(line2);

}


static void render_temp_uptime_page(const Page* page, const HardwareStats* s,
                                    char line1[LCD_COLS + 1], char line2[LCD_COLS + 1])
{
    (void)page;

    if (s->cpu_temp_c > 0.0)
        snprintf(line1, LCD_COLS + 1, "CPU TEMP:%5.1fC", s->cpu_temp_c);
    else
        snprintf(line1, LCD_COLS + 1, "CPU TEMP:  N/A ");

    int h = (int)(s->uptime_seconds / 3600);
    int m = (int)((s->uptime_seconds - h * 3600) / 60);
    int sec = (int)(s->uptime_seconds) % 60;

    snprintf(line2, LCD_COLS + 1, "UP %02d:%02d:%02d", h, m, sec);

    pad16(line1);
    pad16(line2);
}



static Page g_page_cpu = {.name = "CPU", .render = render_cpu_page, .next = NULL, .prev = NULL};


static Page g_page_ram = {.name = "RAM", .render = render_ram_page, .next = NULL, .prev = NULL};

static Page g_page_temp = {.name = "TEMP", .render = render_temp_uptime_page, .next = NULL, .prev = NULL};


static void link_circular_3(Page* a, Page* b, Page* c){

    a->next = b; b->prev = a;
    b->next = c; c->prev = b;
    c->next = a; a->prev = c;

}



int page_manager_init(PageManager* pm){

    if(!pm) return -1;

    memset(pm, 0, sizeof(*pm));

    link_circular_3(&g_page_cpu, &g_page_ram, &g_page_temp);

    pm->head = &g_page_cpu;
    pm->current = pm->head;
    pm->count = 3;

    return 0;

}

void page_manager_deinit(PageManager *pm){

    if(!pm) return;
    pm->head = NULL;
    pm->current = NULL;
    pm->count = 0;

}

void page_manager_next(PageManager* pm){

    if(!pm || !pm->current) return;

    pm->current = pm->current->next;

}

void page_manager_prev(PageManager *pm){

    if(!pm || !pm->current) return;

    pm->current = pm->current->prev;

}


void page_manager_render(const PageManager *pm, const HardwareStats *stats, char line1[LCD_COLS + 1], char line2[LCD_COLS + 1]){

    if(!pm || !pm->current || !pm->current->render || !stats){

        //safe fail

        snprintf(line1, LCD_COLS + 1, "ERR");
        snprintf(line2, LCD_COLS + 1, "NO PAGE/STATS");

        pad16(line1);
        pad16(line2);
        return;

    }


    pm->current->render(pm->current, stats, line1, line2);

    pad16(line1);
    pad16(line2);


}



const char* page_manager_current_name(const PageManager* pm){

    if(!pm || !pm->current) return "NULL";

    return pm->current->name ? pm->current->name : "NONAME";

}