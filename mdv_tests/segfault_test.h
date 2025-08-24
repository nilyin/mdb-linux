#include <minunit.h>
#include <mdv_client.h>
#include <mdv_table.h>
#include <mdv_rowset.h>
#include <mdv_enumerator.h>

void segfault_test(void)
{
    mdv_client_config config = {
        .db = {
            .addr = "tcp://127.0.0.1:4800"
        },
        .connection = {
            .response_timeout = 10,
            .retry_interval = 1,
            .keepidle = 1,
            .keepcnt = 1,
            .keepintvl = 1,
        },
        .threadpool = {
            .size = 2,
        },
    };

    mdv_client *client = mdv_client_connect(&config);
    mu_check(client);

    mdv_field fields[] = {
        { MDV_FLD_TYPE_CHAR, 64, "name" },
        { MDV_FLD_TYPE_UINT32, 1, "age" }
    };

    mdv_table_desc table_desc = {
        .name = "segfault_test",
        .size = sizeof fields / sizeof *fields,
        .fields = fields
    };

    mdv_table *table = mdv_create_table(client, &table_desc);
    mu_check(table);

    // Create a rowset
    mdv_rowset *rowset = mdv_rowset_create(table);
    mu_check(rowset);

    // Try to get an enumerator - this should trigger the segfault
    mdv_enumerator *enumerator = mdv_rowset_enumerator(rowset);
    mu_check(enumerator);
    
    if (enumerator) {
        mdv_enumerator_release(enumerator);
    }
    
    mdv_rowset_release(rowset);
    mdv_table_release(table);
    mdv_client_close(client);
}

MU_TEST_SUITE(segfault_suite)
{
    MU_RUN_TEST(segfault_test);
}