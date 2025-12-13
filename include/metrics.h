#ifndef METRICS_H
#define METRICS_H


typedef struct {

    double cpu_usage_percent;   // the last calculated cpu usage percentage %
    long mem_total_kb;
    long mem_available_kb;
    double load1, load5, load15;
    double uptime_seconds;
    double cpu_temp_c;

}SystemStats;


int read_system_stats(SystemStats* out);

#endif