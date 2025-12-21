#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hardware_stats.h"

static unsigned long long previous_total       = 0;
static unsigned long long previous_idle        = 0;
static                int previous_initialized = 0;


static int read_cpu_times(unsigned long long *total_out, unsigned long long *idle_out){

    FILE* f = fopen("/proc/stat", "r");
    
    if(!f) return -1;

    char line[256];

    if(!fgets(line, sizeof(line), f)) {

        fclose(f);
        return -1;
    }

    fclose(f);

    char cpu_label[4];
    unsigned long long user_state, nice_state, system_state, idle_state, iowait_state, irq_state, softirq_state, steal_state, guest_state, guest_nice_state;

    int count_read = sscanf(line, "%3s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                                   cpu_label, &user_state, &nice_state, &system_state, &idle_state, &iowait_state,
                                   &irq_state, &softirq_state, &steal_state, &guest_state, &guest_nice_state);
    
    if(count_read < 5) return -1;

    unsigned long long non_idle_state = user_state + nice_state + system_state + irq_state + softirq_state + steal_state + guest_state + guest_nice_state;
    unsigned long long idle_all_state = idle_state + iowait_state;
    unsigned long long total_time     = non_idle_state + idle_all_state;

    *total_out = total_time;
    *idle_out  = idle_all_state;


    return 0;

}



static double calc_cpu_usage_time(){

    unsigned long long total = 0;
    unsigned long long idle  = 0;

    if(read_cpu_times(&total, &idle) != 0) return -1.0;

    if(!previous_initialized) {

        previous_total       = total;
        previous_idle        = idle;
        previous_initialized = 1;

        return 0.0;        // first call of this function we give meaningless value. Lets say 0.0 .

    }


    unsigned long long total_diff = total - previous_total;
    unsigned long long idle_diff = idle - previous_idle;

    previous_total = total;
    previous_idle  = idle;

    if(total_diff == 0) return 0.0;

    double usage = 100.0 * (double)(total_diff - idle_diff) / (double)total_diff;

    if(usage < 0.0)   usage = 0.0;

    if(usage > 100.0) usage = 100.0;

    return usage;
}



static int read_memory_info(long* total_kb_out, long* available_kb_out){

    FILE* f = fopen("/proc/meminfo", "r");

    if(!f) return -1;

    char label[64];
    long data;
    char mem_unit[16];


    long mem_total     = -1;
    long mem_available = -1;


    while(fscanf(f, "%63s %ld %15s\n", label, &data, mem_unit) == 3){

        if(strcmp(label, "MemTotal:") == 0) mem_total = data;

        if(strcmp(label, "MemAvailable:") == 0) mem_available = data;

        if(mem_total != -1 && mem_available != -1) break;

    }

    fclose(f);


    *total_kb_out     = mem_total;
    *available_kb_out = mem_available;

    return 0;

}


static int read_load_average(double* l1, double* l5, double* l15){

    FILE* f = fopen("/proc/loadavg", "r");

    if(!f) return -1;

    double load1  = 0.0;
    double load5  = 0.0;
    double load15 = 0.0;


    if(fscanf(f, "%lf %lf %lf", &load1, &load5, &load15) != 3) {

        fclose(f);
        return -1;

    }

    fclose(f);

    *l1  = load1;
    *l5  = load5;
    *l15 = load15;

    return 0;
}


static int read_uptime(double* uptime_second){

    FILE* f = fopen("/proc/uptime", "r");

    if(!f) return -1;

    double up   = 0.0;
    double idle = 0.0;

    if(fscanf(f, "%lf %lf", &up, &idle) != 2){

        fclose(f);
        return -1;

    }

    fclose(f);

    *uptime_second = up;

    return 0;
}


static double read_cpu_tempurature_in_celcius(){

    const char* file_paths_for_temp[] = {"/sys/class/thermal/thermal_zone0/temp", "/sys/class/hwmon/hwmon0/temp1_input"};

    for(size_t i = 0; i < sizeof(file_paths_for_temp) / sizeof(file_paths_for_temp[0]); i++){


        FILE* f = fopen(file_paths_for_temp[i], "r");

        if(!f) continue;

        long milli_celcius = 0;

        if(fscanf(f, "%ld", &milli_celcius) == 1){

            fclose(f);
            return milli_celcius / 1000.0;  // milli celciuse to celcius

        }

        fclose(f);

    }

    return -1.0;
}


int read_system_stats(HardwareStats *out){

    if(!out) return -1;
    
    out->cpu_usage_percent = calc_cpu_usage_time();

    if(read_memory_info(&out->mem_total_kb, &out->mem_available_kb) != 0) return -1;

    if(read_load_average(&out->load1, &out->load5, &out->load15) != 0) return -1;

    if(read_uptime(&out->uptime_seconds) != 0) return -1;

    out->cpu_temp_c = read_cpu_tempurature_in_celcius();

    return 0;
}