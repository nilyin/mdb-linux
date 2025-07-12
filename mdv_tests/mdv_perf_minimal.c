#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <mdv_client.h>
#include <mdv_table.h>
#include <mdv_rowset.h>

double get_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int main() {
    printf("=== MedvedDB Minimal Performance Test ===\n");
    
    // Clean database before test
    printf("🧹 Cleaning database...\n");
    system("rm -rf ./data");
    system("mkdir -p ./data");
    
    // Connect to server
    mdv_client_config config = {
        .db = { .addr = "tcp://127.0.0.1:4800" },
        .connection = { .response_timeout = 10, .retry_interval = 1, .keepidle = 1, .keepcnt = 1, .keepintvl = 1 },
        .threadpool = { .size = 2 }
    };
    
    mdv_client *client = mdv_client_connect(&config);
    if (!client) {
        printf("❌ Failed to connect to MedvedDB server\n");
        return 1;
    }
    printf("✅ Connected to MedvedDB server\n");
    
    // Create table
    mdv_field fields[] = {
        { MDV_FLD_TYPE_CHAR, 64, "name" },
        { MDV_FLD_TYPE_UINT32, 1, "age" }
    };
    
    mdv_table_desc table_desc = {
        .name = "perf_minimal",
        .size = sizeof fields / sizeof *fields,
        .fields = fields
    };
    
    mdv_table *table = mdv_create_table(client, &table_desc);
    if (!table) {
        printf("❌ Failed to create table\n");
        mdv_client_close(client);
        return 1;
    }
    printf("✅ Created table successfully\n");
    
    // Simple insert test
    printf("\n=== Insert Performance Test ===\n");
    double start_time = get_time_ms();
    int insert_count = 100;  // Start small
    int success_count = 0;
    
    for (int i = 0; i < insert_count; i++) {
        mdv_rowset *rowset = mdv_rowset_create(table);
        if (!rowset) {
            continue;
        }
        
        char name[64];
        snprintf(name, sizeof(name), "TestUser_%d", i);
        uint32_t age = 25 + (i % 40);
        
        mdv_data row[] = {
            { .ptr = name, .size = strlen(name) + 1 },
            { .ptr = &age, .size = sizeof(uint32_t) }
        };
        mdv_data const *rows[] = { row };
        mdv_objid row_id = {0};
        
        if (mdv_rowset_append(rowset, &row_id, rows, 1) == 1) {
            if (mdv_insert(client, rowset) == MDV_OK) {
                success_count++;
                // Removed verbose logging during transactions
            }
        }
        
        mdv_rowset_release(rowset);
    }
    
    double end_time = get_time_ms();
    double total_time = end_time - start_time;
    
    printf("\n=== Results ===\n");
    printf("✅ Successfully inserted: %d/%d rows\n", success_count, insert_count);
    printf("⏱️  Total time: %.2f ms\n", total_time);
    printf("⚡ Average per insert: %.3f ms\n", total_time / insert_count);
    printf("🚀 Inserts per second: %.1f\n", (success_count * 1000.0) / total_time);
    
    // Cleanup
    mdv_table_release(table);
    mdv_client_close(client);
    
    printf("\n✅ Performance test completed successfully!\n");
    return 0;
}