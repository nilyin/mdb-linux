#ifndef MDV_PERF_H
#define MDV_PERF_H

#include <stdint.h>
#include <sys/time.h>
#include <sys/resource.h>

// Performance metrics structure
typedef struct {
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    double min_cpu_percent;
    double max_cpu_percent;
    double avg_cpu_percent;
    long min_memory_mb;
    long max_memory_mb;
    long avg_memory_mb;
    int sample_count;
} mdv_perf_metrics;

// System monitoring structure
typedef struct {
    struct timeval start_time;
    struct timeval end_time;
    struct rusage start_usage;
    struct rusage end_usage;
    long start_memory;
    long end_memory;
} mdv_perf_monitor;

// Function declarations
void mdv_perf_monitor_start(mdv_perf_monitor *monitor);
void mdv_perf_monitor_stop(mdv_perf_monitor *monitor);
void mdv_perf_calculate_metrics(mdv_perf_monitor *monitor, mdv_perf_metrics *metrics);
void mdv_perf_update_metrics(mdv_perf_metrics *total, mdv_perf_metrics *sample);
void mdv_perf_print_metrics(const char *operation, mdv_perf_metrics *metrics);
void mdv_perf_print_summary_table(void);
long mdv_perf_get_memory_usage(void);

// Performance test functions
void mdv_perf_test_bulk_inserts(void);
void mdv_perf_test_single_inserts(void);
void mdv_perf_test_single_updates(void);
void mdv_perf_test_bulk_updates(void);
void mdv_perf_test_bulk_reads(void);
void mdv_perf_test_single_reads(void);
void mdv_perf_test_single_deletes(void);
void mdv_perf_test_delete_all(void);

// Main performance test runner
void mdv_run_performance_tests(void);

#endif // MDV_PERF_H