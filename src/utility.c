#include <stdio.h>
#include "metrics.h"
#include "utility.h"
#include <unistd.h>

static void print_stats(SystemStats* s){

    double used_memory_in_mb  = (s->mem_total_kb - s->mem_available_kb) / 1024.0;
    double total_memory_in_mb = s->mem_total_kb / 1024.0;


    int hours = (int)(s->uptime_seconds / 3600);
    int mins  = (int)((s->uptime_seconds - hours * 3600) / 60);
    int secs  = (int)((s->uptime_seconds - hours * 3600 - mins * 60));

    printf("--------------------------------------------------\n");
    printf("CPU Usage   : %5.1f %%\n", s->cpu_usage_percent);
    printf("Memory      : %6.1f / %6.1f MB (used/total)\n", used_memory_in_mb, total_memory_in_mb);
    printf("Load Average: %.2f  %.2f  %.2f\n", s->load1, s->load5, s->load15);
    printf("Uptime      : %02d:%02d:%02d\n", hours, mins, secs);
    
    if(s->cpu_temp_c > 0.0) printf("CPU Temp    : %.1f C\n", s->cpu_temp_c);

    else printf("CPU Temp    : N/A\n");

    printf("--------------------------------------------------\n");
}



int display_stats_only_terminal(SystemStats* s){

    while(1){

            if(read_system_stats(s) != 0){

                fprintf(stderr, "read_system_stats failed!\n");
                return 1;
            }

            print_stats(s);

            sleep(1);

    }

    return 0;
}