#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <mdv_client.h>
#include <mdv_table.h>
#include <mdv_rowset.h>
#include <mdv_enumerator.h>

static mdv_client *g_client = NULL;
static mdv_table *g_table = NULL;

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int setup_test_environment() {
    mdv_client_config config = {
        .db = { .addr = "tcp://127.0.0.1:4800" },
        .connection = { .response_timeout = 10, .retry_interval = 1, .keepidle = 1, .keepcnt = 1, .keepintvl = 1 },
        .threadpool = { .size = 2 }
    };
    
    g_client = mdv_client_connect(&config);
    if (!g_client) {
        printf("Failed to connect to MedvedDB server\n");
        return 0;
    }
    
    mdv_field fields[] = {
        { MDV_FLD_TYPE_CHAR, 64, "name" },
        { MDV_FLD_TYPE_UINT32, 1, "age" }
    };
    
    mdv_table_desc table_desc = {
        .name = "perf_simple",
        .size = sizeof fields / sizeof *fields,
        .fields = fields
    };
    
    g_table = mdv_create_table(g_client, &table_desc);
    if (!g_table) {
        printf("Failed to create table\n");
        return 0;
    }
    
    printf("Test environment setup successful\n");
    return 1;
}

void cleanup_test_environment() {
    if (g_table) mdv_table_release(g_table);
    if (g_client) mdv_client_close(g_client);
}

void test_simple_inserts() {
    printf("\n=== Simple Insert Test ===\n");
    
    double start_time = get_time_ms();
    int insert_count = 1000;
    int success_count = 0;
    
    for (int i = 0; i < insert_count; i++) {
        mdv_rowset *rowset = mdv_rowset_create(g_table);
        if (!rowset) continue;
        
        char name[64];
        snprintf(name, sizeof(name), "User_%d", i);
        
        mdv_data row[] = {
            { .ptr = name, .size = strlen(name) + 1 },
            { .ptr = &(uint32_t){ 25 + (i % 40) }, .size = sizeof(uint32_t) }
        };
        mdv_data const *rows[] = { row };
        
        if (mdv_rowset_append(rowset, NULL, rows, 1) == 1) {
            if (mdv_insert(g_client, rowset) == MDV_OK) {
                success_count++;
            }
        }
        
        mdv_rowset_release(rowset);
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    
    printf("✅ Insert Test: %d/%d rows, %.2f ms avg, %.1f ops/sec\n", 
           success_count, insert_count, total_time / insert_count, (success_count * 1000.0) / total_time);
}

void test_simple_reads() {
    printf("\n=== Simple Read Test ===\n");
    
    double start_time = get_time_ms();
    int read_count = 100;
    int success_count = 0;
    
    for (int i = 0; i < read_count; i++) {
        mdv_rowset *rowset = mdv_client_select(g_client, g_table, NULL, "");
        if (rowset) {
            mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
            if (enumerator) {
                int rows = 0;
                while (mdv_enumerator_next(enumerator) == MDV_OK) {
                    rows++;
                    if (rows >= 10) break; // Limit to first 10 rows
                }
                if (rows > 0) success_count++;
                mdv_enumerator_release(enumerator);
            }
            mdv_rowset_release(rowset);
        }
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    
    printf("✅ Read Test: %d/%d ops, %.3f ms avg, %.1f ops/sec\n", 
           success_count, read_count, total_time / read_count, (success_count * 1000.0) / total_time);
}

int main() {
    printf("=== MedvedDB Simple Performance Test ===\n");
    
    if (!setup_test_environment()) {
        printf("Failed to setup test environment\n");
        return 1;
    }
    
    test_simple_inserts();
    test_simple_reads();
    
    cleanup_test_environment();
    
    printf("\n=== Performance Test Completed ===\n");
    return 0;
}