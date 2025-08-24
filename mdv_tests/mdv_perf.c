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

// DEFAULT_PERF_CONFIG is provided as a macro in perf_config.h.
// Initialize g_config at runtime in mdv_run_performance_tests().
static mdv_client *g_client = NULL;
static mdv_table *g_table = NULL;
static mdv_perf_config g_config;
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

void mdv_perf_print_metrics(const char *operation, mdv_perf_metrics *metrics, int operation_count, int error_count) {
    double per_op_time = metrics->avg_time_ms / operation_count;
    printf("✅ %s: %.2f ms total (%.4f ms per operation, %.2f%% CPU, %ld MB, %d errors)\n", 
           operation, metrics->avg_time_ms, per_op_time, metrics->avg_cpu_percent, metrics->avg_memory_mb, error_count);
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
        { MDV_FLD_TYPE_CHAR, 0, "name" },
        { MDV_FLD_TYPE_UINT32, 4, "age" },
        { MDV_FLD_TYPE_UINT64, 8, "timestamp" }
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
    int error_count = 0;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            
            for (int i = 0; i < g_config.bulk_batch_size; i++) {
                char name[256] = {0};
                snprintf(name, sizeof(name), "User_%d_%d", batch, i);
                uint64_t timestamp = (uint64_t)time(NULL) + batch * 1000 + i;
                
                mdv_data row[] = {
                    { .ptr = name, .size = strlen(name) + 1 },
                    { .ptr = &(uint32_t){ 20 + (i % 50) }, .size = 4 },
                    { .ptr = &timestamp, .size = 8 }
                };
                mdv_data const *rows[] = { row };
                mdv_rowset_append(rowset, rows, 1);
            }
            
            if (mdv_insert(g_client, rowset) != MDV_OK) error_count++;
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[0] = total_metrics;
    mdv_perf_print_metrics("Bulk Inserts", &total_metrics, g_config.bulk_total_inserts, error_count);
}

void mdv_perf_test_single_inserts(void) {
    mdv_perf_metrics total_metrics = {0};
    int error_count = 0;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_inserts; i++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            
            char name[256] = {0};
            snprintf(name, sizeof(name), "SingleUser_%d", i);
            uint64_t timestamp = (uint64_t)time(NULL) + i;
            
            mdv_data row[] = {
                { .ptr = name, .size = strlen(name) + 1 },
                { .ptr = &(uint32_t){ 25 + (i % 40) }, .size = 4 },
                { .ptr = &timestamp, .size = 8 }
            };
            mdv_data const *rows[] = { row };
            
            mdv_rowset_append(rowset, rows, 1);
            if (mdv_insert(g_client, rowset) != MDV_OK) error_count++;
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[1] = total_metrics;
    mdv_perf_print_metrics("Single Inserts", &total_metrics, g_config.single_total_inserts, error_count);
}

void mdv_perf_test_single_updates(void) {
    mdv_perf_metrics total_metrics = {0};
    int error_count = 0;
    
    // Get some row IDs first
    mdv_rowset *select_rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
    if (!select_rowset) {
        printf("Failed to select rows for updates\n");
        return;
    }
    
    mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
    if (!enumerator) {
        mdv_rowset_release(select_rowset);
        printf("Failed to create enumerator for updates\n");
        return;
    }
    
    mdv_objid *row_ids = malloc(g_config.single_total_updates * sizeof(mdv_objid));
    int row_count = 0;
    
    while (mdv_enumerator_next(enumerator) == MDV_OK && row_count < g_config.single_total_updates) {
        const mdv_objid *id = mdv_enumerator_row_id(enumerator);
        if (id) {
            row_ids[row_count++] = *id;
        }
    }
    
    mdv_enumerator_release(enumerator);
    mdv_rowset_release(select_rowset);
    
    if (row_count == 0) {
        printf("No rows found for updates\n");
        free(row_ids);
        return;
    }
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < row_count; i++) {
            mdv_rowset *rowset = mdv_rowset_create(g_table);
            if (!rowset) {
                error_count++;
                continue;
            }
            
            char name[256] = {0};
            snprintf(name, sizeof(name), "UpdatedUser_%d", i);
            uint64_t timestamp = (uint64_t)time(NULL) + i + 1000000;
            
            mdv_data row[] = {
                { .ptr = name, .size = strlen(name) + 1 },
                { .ptr = &(uint32_t){ 30 + (i % 35) }, .size = 4 },
                { .ptr = &timestamp, .size = 8 }
            };
            mdv_data const *rows[] = { row };
            
            mdv_rowset_append(rowset, rows, 1);
            if (mdv_update(g_client, g_table, &row_ids[i], rowset) != MDV_OK) error_count++;
            mdv_rowset_release(rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    free(row_ids);
    g_test_results[2] = total_metrics;
    mdv_perf_print_metrics("Single Updates", &total_metrics, row_count, error_count);
}

void mdv_perf_test_bulk_updates(void) {
    mdv_perf_metrics total_metrics = {0};
    int batches = g_config.bulk_total_updates / g_config.bulk_batch_size;
    int error_count = 0;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *select_rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
            
            int updates = 0;
            
            while (mdv_enumerator_next(enumerator) == MDV_OK && updates < g_config.bulk_batch_size) {
                const mdv_objid *id = mdv_enumerator_row_id(enumerator);
                
                mdv_rowset *update_rowset = mdv_rowset_create(g_table);
                
                char name[256] = {0};
                snprintf(name, sizeof(name), "BulkUpdate_%d_%d", batch, updates);
                uint64_t timestamp = (uint64_t)time(NULL) + batch * 10000 + updates;
                
                mdv_data row[] = {
                    { .ptr = name, .size = strlen(name) + 1 },
                    { .ptr = &(uint32_t){ 35 + (updates % 30) }, .size = 4 },
                    { .ptr = &timestamp, .size = 8 }
                };
                mdv_data const *rows[] = { row };
                
                mdv_rowset_append(update_rowset, rows, 1);
                if (mdv_update(g_client, g_table, id, update_rowset) != MDV_OK) error_count++;
                mdv_rowset_release(update_rowset);
                updates++;
            }
            
            mdv_enumerator_release(enumerator);
            mdv_rowset_release(select_rowset);
        }
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    g_test_results[3] = total_metrics;
    mdv_perf_print_metrics("Bulk Updates", &total_metrics, g_config.bulk_total_updates, error_count);
}

void mdv_perf_test_bulk_reads(void) {
    mdv_perf_metrics total_metrics = {0};
    int batches = g_config.bulk_total_reads / g_config.bulk_batch_size;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int batch = 0; batch < batches; batch++) {
            mdv_rowset *rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
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
    mdv_perf_print_metrics("Bulk Reads", &total_metrics, g_config.bulk_total_reads, 0);
}

void mdv_perf_test_single_reads(void) {
    mdv_perf_metrics total_metrics = {0};
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_reads; i++) {
            mdv_rowset *rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
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
    mdv_perf_print_metrics("Single Reads", &total_metrics, g_config.single_total_reads, 0);
}

void mdv_perf_test_single_deletes(void) {
    mdv_perf_metrics total_metrics = {0};
    int error_count = 0;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        for (int i = 0; i < g_config.single_total_deletes; i++) {
            mdv_rowset *rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
            mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
            
            if (mdv_enumerator_next(enumerator) == MDV_OK) {
                const mdv_objid *id = mdv_enumerator_row_id(enumerator);
                if (mdv_delete(g_client, g_table, id) != MDV_OK) error_count++;
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
    mdv_perf_print_metrics("Single Deletes", &total_metrics, g_config.single_total_deletes, error_count);
}

void mdv_perf_test_delete_all(void) {
    mdv_perf_metrics total_metrics = {0};
    int error_count = 0;
    
    for (int sample = 0; sample < g_config.measurement_samples; sample++) {
        mdv_perf_monitor monitor;
        mdv_perf_monitor_start(&monitor);
        
        mdv_rowset *rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
        mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
        
        while (mdv_enumerator_next(enumerator) == MDV_OK) {
            const mdv_objid *id = mdv_enumerator_row_id(enumerator);
            if (mdv_delete(g_client, g_table, id) != MDV_OK) error_count++;
        }
        
        mdv_enumerator_release(enumerator);
        mdv_rowset_release(rowset);
        
        mdv_perf_monitor_stop(&monitor);
        mdv_perf_metrics sample_metrics;
        mdv_perf_calculate_metrics(&monitor, &sample_metrics);
        mdv_perf_update_metrics(&total_metrics, &sample_metrics);
    }
    
    // Count remaining rows for delete all
    int delete_count = 0;
    mdv_rowset *count_rowset = mdv_dbclient_select(g_client, g_table, NULL, "");
    if (count_rowset) {
        mdv_enumerator *count_enum = mdv_rowset_enumerator(count_rowset);
        while (mdv_enumerator_next(count_enum) == MDV_OK) delete_count++;
        mdv_enumerator_release(count_enum);
        mdv_rowset_release(count_rowset);
    }
    
    g_test_results[7] = total_metrics;
    mdv_perf_print_metrics("Delete All", &total_metrics, delete_count > 0 ? delete_count : 1, error_count);
}

void mdv_perf_print_summary_table(void) {
    // Operation counts for per-operation calculations
    int op_counts[] = {
        g_config.bulk_total_inserts,    // Bulk Inserts
        g_config.single_total_inserts,  // Single Inserts  
        g_config.single_total_updates,  // Single Updates
        g_config.bulk_total_updates,    // Bulk Updates
        g_config.bulk_total_reads,      // Bulk Reads
        g_config.single_total_reads,    // Single Reads
        g_config.single_total_deletes,  // Single Deletes
        1                               // Delete All (estimated)
    };
    
    printf("\n\n=== PERFORMANCE TEST SUMMARY TABLE ===\n");
    printf("%-15s | %-12s | %-12s | %-12s | %-12s | %-10s | %-10s | %-10s | %-10s | %-10s | %-10s\n",
           "Operation", "Per Op(ms)", "Min Time(ms)", "Max Time(ms)", "Avg Time(ms)", 
           "Min CPU(%)", "Max CPU(%)", "Avg CPU(%)", "Min Mem(MB)", "Max Mem(MB)", "Avg Mem(MB)");
    printf("----------------+-------------+-------------+-------------+-------------+-----------+-----------+-----------+-----------+-----------+-----------\n");
    
    for (int i = 0; i < 8; i++) {
        mdv_perf_metrics *m = &g_test_results[i];
        double per_op_time = m->avg_time_ms / op_counts[i];
        printf("%-15s | %11.4f | %11.2f | %11.2f | %11.2f | %9.2f | %9.2f | %9.2f | %9ld | %9ld | %9ld\n",
               g_test_names[i], per_op_time, m->min_time_ms, m->max_time_ms, m->avg_time_ms,
               m->min_cpu_percent, m->max_cpu_percent, m->avg_cpu_percent,
               m->min_memory_mb, m->max_memory_mb, m->avg_memory_mb);
    }
    printf("\n");
}

void mdv_run_performance_tests(void) {
    printf("=== MedvedDB Performance Test Suite ===\n");

    /* Initialize runtime configuration from the macro-based DEFAULT_PERF_CONFIG.
       Compound literals/macros cannot be used as compile-time initializers for
       objects with static storage duration, so we perform assignment at runtime. */
    g_config = DEFAULT_PERF_CONFIG;
    
    // Clean database before test
    printf("🧹 Cleaning database...\n");
    system("rm -rf ./data");
    system("mkdir -p ./data");
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