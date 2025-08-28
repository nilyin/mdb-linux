#include <mdv_client.h>
#include <mdv_table.h>
#include <mdv_rowset.h>
#include <mdv_enumerator.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== Debug SELECT Test ===\n");
    
    mdv_client_config config = {
        .db = { .addr = "tcp://127.0.0.1:4800" },
        .connection = { .response_timeout = 10, .retry_interval = 1, .keepidle = 1, .keepcnt = 1, .keepintvl = 1 },
        .threadpool = { .size = 2 }
    };
    
    mdv_client *client = mdv_client_connect(&config);
    if (!client) {
        printf("❌ Failed to connect to server\n");
        return 1;
    }
    
    mdv_field fields[] = {
        { MDV_FLD_TYPE_CHAR, 0, "name" },
        { MDV_FLD_TYPE_UINT32, 4, "age" },
        { MDV_FLD_TYPE_UINT64, 8, "timestamp" }
    };
    
    mdv_table_desc table_desc = {
        .name = "debug_test",
        .size = sizeof fields / sizeof *fields,
        .fields = fields
    };
    
    mdv_table *table = mdv_create_table(client, &table_desc);
    if (!table) {
        printf("❌ Failed to create table\n");
        mdv_client_close(client);
        return 1;
    }
    
    printf("✅ Connected and created table\n");
    
    // Insert one test row
    mdv_rowset *insert_rowset = mdv_rowset_create(table);
    char name[] = "TestUser";
    uint32_t age = 25;
    uint64_t timestamp = 1234567890;
    
    mdv_data row[] = {
        { .ptr = name, .size = strlen(name) + 1 },
        { .ptr = &age, .size = 4 },
        { .ptr = &timestamp, .size = 8 }
    };
    mdv_data const *rows[] = { row };
    
    mdv_rowset_append(insert_rowset, rows, 1);
    if (mdv_insert(client, insert_rowset) != MDV_OK) {
        printf("❌ Failed to insert test row\n");
    } else {
        printf("✅ Inserted test row\n");
    }
    mdv_rowset_release(insert_rowset);
    
    // Wait a moment for data to be committed
    sleep(1);
    
    // Try to SELECT the data
    printf("🔍 Attempting SELECT operation...\n");
    mdv_rowset *select_rowset = mdv_dbclient_select(client, table, NULL, "");
    if (!select_rowset) {
        printf("❌ SELECT failed - returned NULL rowset\n");
        mdv_table_release(table);
        mdv_client_close(client);
        return 1;
    }
    
    printf("✅ SELECT returned rowset\n");
    
    // Try to enumerate the results
    mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
    if (!enumerator) {
        printf("❌ Failed to create enumerator\n");
        mdv_rowset_release(select_rowset);
        mdv_table_release(table);
        mdv_client_close(client);
        return 1;
    }
    
    printf("✅ Created enumerator\n");
    
    int row_count = 0;
    while (mdv_enumerator_next(enumerator) == MDV_OK) {
        const mdv_objid *id = mdv_enumerator_row_id(enumerator);
        if (id) {
            printf("✅ Found row with ID: node=%u, id=%llu\n", id->node, id->id);
        } else {
            printf("⚠️  Found row but no row ID\n");
        }
        row_count++;
    }
    
    printf("📊 Total rows found: %d\n", row_count);
    
    mdv_enumerator_release(enumerator);
    mdv_rowset_release(select_rowset);
    mdv_table_release(table);
    mdv_client_close(client);
    
    if (row_count > 0) {
        printf("🎉 SUCCESS: SELECT operation working correctly\n");
        return 0;
    } else {
        printf("❌ FAILURE: No rows found in SELECT\n");
        return 1;
    }
}