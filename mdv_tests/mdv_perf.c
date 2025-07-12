#include "mdv_perf.h"
#include "perf_config.h"
#include <minunit.h>
#include <mdv_client.h>
#include <mdv_log.h>
#include <mdv_table.h>
#include <mdv_rowset.h>
#include <mdv_enumerator.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static mdv_client *g_client = NULL;
static mdv_table *g_table = NULL;
static mdv_perf_config g_config = DEFAULT_PERF_CONFIG;
static mdv_perf_metrics g_test_results[8];
static const char* g_test_names[] = {
    "Bulk Inserts", "Single Inserts", "Single Updates", "Bulk Updates",
    "Bulk Reads", "Single Reads", "Single Deletes", "Delete All"
};

long mdv_perf_get_memory_usage(void) {
    FILE *file = fopen("/proc/self/status", "r");
    if (!file) return 0;
    
    char line[128];
    long memory_kb = 0;
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line, "VmRSS: %ld kB", &memory_kb);
            break;
        }
    }
    fclose(file);
    return memory_kb / 1024; // Convert to MB
}

void mdv_perf_monitor_start(mdv_perf_monitor *monitor) {
    gettimeofday(&monitor->start_time, NULL);
    getrusage(RUSAGE_SELF, &monitor->start_usage);
    monitor->start_memory = mdv_perf_get_memory_usage();
}

void mdv_perf_monitor_stop(mdv_perf_monitor *monitor) {
    gettimeofday(&monitor->end_time, NULL);
    getrusage(RUSAGE_SELF, &monitor->end_usage);
    monitor->end_memory = mdv_perf_get_memory_usage();
}

void mdv_perf_calculate_metrics(mdv_perf_monitor *monitor, mdv_perf_metrics *metrics) {
    // Calculate time in milliseconds
    double time_ms = (monitor->end_time.tv_sec - monitor->start_time.tv_sec) * 1000.0 +
                     (monitor->end_time.tv_usec - monitor->start_time.tv_usec) / 1000.0;
    
    // Calculate CPU usage percentage
    double user_time = (monitor->end_usage.ru_utime.tv_sec - monitor->start_usage.ru_utime.tv_sec) +
                       (monitor->end_usage.ru_utime.tv_usec - monitor->start_usage.ru_utime.tv_usec) / 1000000.0;
    double sys_time = (monitor->end_usage.ru_stime.tv_sec - monitor->start_usage.ru_stime.tv_sec) +
                      (monitor->end_usage.ru_stime.tv_usec - monitor->start_usage.ru_stime.tv_usec) / 1000000.0;
    double cpu_percent = ((user_time + sys_time) / (time_ms / 1000.0)) * 100.0;
    
    metrics->min_time_ms = metrics->max_time_ms = metrics->avg_time_ms = time_ms;
    metrics->min_cpu_percent = metrics->max_cpu_percent = metrics->avg_cpu_percent = cpu_percent;
    metrics->min_memory_mb = metrics->max_memory_mb = metrics->avg_memory_mb = monitor->end_memory;
    metrics->sample_count = 1;
}

void mdv_perf_update_metrics(mdv_perf_metrics *total, mdv_perf_metrics *sample) {
    if (total->sample_count == 0) {
        *total = *sample;
        return;
    }
    
    // Update min/max values
    if (sample->min_time_ms < total->min_time_ms) total->min_time_ms = sample->min_time_ms;
    if (sample->max_time_ms > total->max_time_ms) total->max_time_ms = sample->max_time_ms;
    if (sample->min_cpu_percent < total->min_cpu_percent) total->min_cpu_percent = sample->min_cpu_percent;
    if (sample->max_cpu_percent > total->max_cpu_percent) total->max_cpu_percent = sample->max_cpu_percent;
    if (sample->min_memory_mb < total->min_memory_mb) total->min_memory_mb = sample->min_memory_mb;
    if (sample->max_memory_mb > total->max_memory_mb) total->max_memory_mb = sample->max_memory_mb;
    
    // Update averages
    total->avg_time_ms = (total->avg_time_ms * total->sample_count + sample->avg_time_ms) / (total->sample_count + 1);
    total->avg_cpu_percent = (total->avg_cpu_percent * total->sample_count + sample->avg_cpu_percent) / (total->sample_count + 1);
    total->avg_memory_mb = (total->avg_memory_mb * total->sample_count + sample->avg_memory_mb) / (total->sample_count + 1);
    
    total->sample_count++;
}

void mdv_perf_print_metrics(const char *operation, mdv_perf_metrics *metrics) {
    printf("\n=== %s Performance Results ===\n", operation);
    printf("Time (ms):   Min: %.2f, Max: %.2f, Avg: %.2f\n", 
           metrics->min_time_ms, metrics->max_time_ms, metrics->avg_time_ms);
    printf("CPU (%%):     Min: %.2f, Max: %.2f, Avg: %.2f\n", 
           metrics->min_cpu_percent, metrics->max_cpu_percent, metrics->avg_cpu_percent);
    printf("Memory (MB): Min: %ld, Max: %ld, Avg: %ld\n", 
           metrics->min_memory_mb, metrics->max_memory_mb, metrics->avg_memory_mb);
    printf("Samples: %d\n", metrics->sample_count);
}

static int setup_test_environment(void) {
    mdv_client_config config = {
        .db = { .addr = "tcp://127.0.0.1:4800" },
        .connection = { .response_timeout = 10, .retry_interval = 1, .keepidle = 1, .keepcnt = 1, .keepintvl = 1 },
        .threadpool = { .size = 2 }
    };
    
    g_client = mdv_client_connect(&config);
    if (!g_client) return 0;
    
    mdv_field fields[] = {
        { MDV_FLD_TYPE_CHAR, 64, "name" },
        { MDV_FLD_TYPE_UINT32, 1, "age" },
        { MDV_FLD_TYPE_UINT64, 1, "timestamp" }
    };
    
    mdv_table_desc table_desc = {
        .name = "perf_test",
        .size = sizeof fields / sizeof *fields,
        .fields = fields
    };
    
    g_table = mdv_create_table(g_client, &table_desc);
    return g_table != NULL;
}

static void cleanup_test_environment(void) {
    if (g_table) mdv_table_release(g_table);
    if (g_client) mdv_client_close(g_client);
}

void mdv_perf_test_bulk_inserts(void) {
    mdv_perf_metrics total_metrics = {0};
    int batches = g_config.bulk_total_inserts / g_config.bulk_batch_size;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            
            for (int i = 0; i < g_config.bulk_batch_size; i++) {
                char name[64];
                snprintf(name, sizeof(name), "User_%d_%d", batch, i);
                uint64_t timestamp = (uint64_t)time(NULL) + batch * 1000 + i;
                
                mdv_data row[] = {
                    { .ptr = name, .size = strlen(name) + 1 },
                    { .ptr = &(uint32_t){ 20 + (i % 50) }, .size = sizeof(uint32_t) },
                    { .ptr = &timestamp, .size = sizeof(uint64_t) }
                };
                mdv_data const *rows[] = { row };
                mdv_rowset_append(rowset, NULL, rows, 1);
            }
            
            mdv_insert(g_client, rowset);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[0] = total_metrics;
    mdv_perf_print_metrics("Bulk Inserts", &total_metrics);
}

void mdv_perf_test_single_inserts(void) {
    mdv_perf_metrics total_metrics = {0};
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_inserts; i++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            
            char name[64];
            snprintf(name, sizeof(name), "SingleUser_%d", i);
            uint64_t timestamp = (uint64_t)time(NULL) + i;
            
            mdv_data row[] = {
                { .ptr = name, .size = strlen(name) + 1 },
                { .ptr = &(uint32_t){ 25 + (i % 40) }, .size = sizeof(uint32_t) },
                { .ptr = &timestamp, .size = sizeof(uint64_t) }
            };
            mdv_data const *rows[] = { row };
            
            mdv_rowset_append(rowset, NULL, rows, 1);
            mdv_insert(g_client, rowset);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[1] = total_metrics;
    mdv_perf_print_metrics("Single Inserts", &total_metrics);
}

void mdv_perf_test_single_updates(void) {
    mdv_perf_metrics total_metrics = {0};
    
    // Get some row IDs first
    mdv_rowset *select_rowset = mdv_client_select(g_client, g_table, NULL, "");
    mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
    
    mdv_objid *row_ids = malloc(g_config.single_total_updates * sizeof(mdv_objid));
    int row_count = 0;
    
    while (mdv_enumerator_next(enumerator) == MDV_OK && row_count < g_config.single_total_updates) {
        row_ids[row_count++] = *mdv_enumerator_row_id(enumerator);
    }
    
    mdv_enumerator_release(enumerator);
    mdv_rowset_release(select_rowset);
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < row_count; i++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            
            char name[64];
            snprintf(name, sizeof(name), "UpdatedUser_%d", i);
            uint64_t timestamp = (uint64_t)time(NULL) + i + 1000000;
            
            mdv_data row[] = {
                { .ptr = name, .size = strlen(name) + 1 },
                { .ptr = &(uint32_t){ 30 + (i % 35) }, .size = sizeof(uint32_t) },
                { .ptr = &timestamp, .size = sizeof(uint64_t) }
            };
            mdv_data const *rows[] = { row };
            
            mdv_rowset_append(rowset, NULL, rows, 1);
            mdv_update(g_client, g_table, &row_ids[i], rowset);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    free(row_ids);
    g_test_results[2] = total_metrics;
    mdv_perf_print_metrics("Single Updates", &total_metrics);
}

void mdv_perf_test_bulk_updates(void) {
    mdv_perf_metrics total_metrics = {0};
    int batches = g_config.bulk_total_updates / g_config.bulk_batch_size;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *select_rowset = mdv_client_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
            
            mdv_rowset *update_rowset = mdv_rowset_create(g_table);
            int updates = 0;
            
            while (mdv_enumerator_next(enumerator) == MDV_OK && updates < g_config.bulk_batch_size) {
                mdv_objid row_id = *mdv_enumerator_row_id(enumerator);
                
                char name[64];
                snprintf(name, sizeof(name), "BulkUpdate_%d_%d", batch, updates);
                uint64_t timestamp = (uint64_t)time(NULL) + batch * 10000 + updates;
                
                mdv_data row[] = {
                    { .ptr = name, .size = strlen(name) + 1 },
                    { .ptr = &(uint32_t){ 35 + (updates % 30) }, .size = sizeof(uint32_t) },
                    { .ptr = &timestamp, .size = sizeof(uint64_t) }
                };
                mdv_data const *rows[] = { row };
                
                mdv_rowset_append(update_rowset, NULL, rows, 1);
                mdv_update(g_client, g_table, &row_id, update_rowset);
                updates++;
            }
            
            mdv_enumerator_release(enumerator);
            mdv_rowset_release(select_rowset);
            mdv_rowset_release(update_rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[3] = total_metrics;
    mdv_perf_print_metrics("Bulk Updates", &total_metrics);
}

void mdv_perf_test_bulk_reads(void) {
    mdv_perf_metrics total_metrics = {0};
    int batches = g_config.bulk_total_reads / g_config.bulk_batch_size;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *rowset = mdv_client_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
            
            int count = 0;
            while (mdv_enumerator_next(enumerator) == MDV_OK && count < g_config.bulk_batch_size) {
                count++;
            }
            
            mdv_enumerator_release(enumerator);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[4] = total_metrics;
    mdv_perf_print_metrics("Bulk Reads", &total_metrics);
}

void mdv_perf_test_single_reads(void) {
    mdv_perf_metrics total_metrics = {0};
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_reads; i++) {
            mdv_rowset *rowset = mdv_client_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
            
            if (mdv_enumerator_next(enumerator) == MDV_OK) {
                // Read operation completed
            }
            
            mdv_enumerator_release(enumerator);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[5] = total_metrics;
    mdv_perf_print_metrics("Single Reads", &total_metrics);
}

void mdv_perf_test_single_deletes(void) {
    mdv_perf_metrics total_metrics = {0};
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_deletes; i++) {
            mdv_rowset *rowset = mdv_client_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
            
            if (mdv_enumerator_next(enumerator) == MDV_OK) {
                mdv_objid row_id = *mdv_enumerator_row_id(enumerator);
                mdv_delete(g_client, g_table, &row_id);
            }
            
            mdv_enumerator_release(enumerator);
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[6] = total_metrics;
    mdv_perf_print_metrics("Single Deletes", &total_metrics);
}

void mdv_perf_test_delete_all(void) {
    mdv_perf_metrics total_metrics = {0};
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        mdv_rowset *rowset = mdv_client_select(g_client, g_table, NULL, "");
        mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
        
        while (mdv_enumerator_next(enumerator) == MDV_OK) {
            mdv_objid row_id = *mdv_enumerator_row_id(enumerator);
            mdv_delete(g_client, g_table, &row_id);
        }
        
        mdv_enumerator_release(enumerator);
        mdv_rowset_release(rowset);
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[7] = total_metrics;
    mdv_perf_print_metrics("Delete All", &total_metrics);
}

void mdv_perf_print_summary_table(void) {
    printf("\n\n=== PERFORMANCE TEST SUMMARY TABLE ===\n");
    printf("%-15s | %-12s | %-12s | %-12s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s\n",
           "Operation", "Min Time(ms)", "Max Time(ms)", "Avg Time(ms)", 
           "Min CPU(%)", "Max CPU(%)", "Avg CPU(%)", "Min Mem(MB)", "Max Mem(MB)", "Avg Mem(MB)");
    printf("----------------+-------------+-------------+-------------+-----------+-----------+-----------+-----------+-----------+-----------\n");
    
    for (int i = 0; i < 8; i++) {
        mdv_perf_metrics *m = &g_test_results[i];
        printf("%-15s | %11.2f | %11.2f | %11.2f | %9.2f | %9.2f | %9.2f | %9ld | %9ld | %9ld\n",
               g_test_names[i], m->min_time_ms, m->max_time_ms, m->avg_time_ms,
               m->min_cpu_percent, m->max_cpu_percent, m->avg_cpu_percent,
               m->min_memory_mb, m->max_memory_mb, m->avg_memory_mb);
    }
    printf("\n");
}

void mdv_run_performance_tests(void) {
    printf("=== MedvedDB Performance Test Suite ===\n");
    printf("Configuration:\n");
    printf("  Bulk batch size: %d\n", g_config.bulk_batch_size);
    printf("  Bulk inserts: %d\n", g_config.bulk_total_inserts);
    printf("  Single inserts: %d\n", g_config.single_total_inserts);
    printf("  Measurement samples: %d\n", g_config.measurement_samples);
    printf("\n");
    
    if (!setup_test_environment()) {
        printf("Failed to setup test environment\n");
        return;
    }
    
    mdv_perf_test_bulk_inserts();
    mdv_perf_test_single_inserts();
    mdv_perf_test_single_updates();
    mdv_perf_test_bulk_updates();
    mdv_perf_test_bulk_reads();
    mdv_perf_test_single_reads();
    mdv_perf_test_single_deletes();
    mdv_perf_test_delete_all();
    
    mdv_perf_print_summary_table();
    cleanup_test_environment();
}