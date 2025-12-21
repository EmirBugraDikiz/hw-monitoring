#ifndef HARDWARE_STATS_H
#define HARDWARE_STATS_H


// struct data to hold system stats. 
// read_system_stats(SystemStats* out) function reads system stats with the help of the functions , which have static linkage, defined inside hardware_stats.c


typedef struct HardwareStats {

    double cpu_usage_percent;   // the last calculated cpu usage percentage %
    long mem_total_kb;
    long mem_available_kb;
    double load1, load5, load15;
    double uptime_seconds;
    double cpu_temp_c;

}HardwareStats;


int read_system_stats(HardwareStats* out);

#endif