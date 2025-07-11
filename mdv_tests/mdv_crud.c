#include "mdv_crud.h"
#include <minunit.h>
#include <mdv_client.h>
#include <mdv_log.h>
#include <mdv_table.h>
#include <mdv_rowset.h>


MU_TEST(crud, create_read_update_delete)
{
    mdv_client_config config =
    {
        .db =
        {
            .path = ".mdb",
            .name = "crud",
        },
        .connection =
        {
            .response_timeout = 10,
            .retry_interval = 1,
            .keepidle = 1,
            .keepcnt = 1,
            .keepintvl = 1,
        },
        .threadpool =
        {
            .size = 2,
        },
    };

    mdv_client *client = mdv_client_connect(&config);
    mu_check(client);

    mdv_field fields[] =
    {
        { "name", MDV_FLD_TYPE_CHAR(64) },
        { "age",  MDV_FLD_TYPE_UINT32    }
    };

    mdv_table_desc table_desc =
    {
        .name = "users",
        .fields =
        {
            .count = sizeof fields / sizeof *fields,
            .entries = fields
        }
    };

    mdv_table *table = mdv_create_table(client, &table_desc);
    mu_check(table);

    // Create
    mdv_rowset *insert_rowset = mdv_rowset_create(table);
    mu_check(insert_rowset);

    mdv_data row[] =
    {
        { .ptr = "John Doe", .size = sizeof("John Doe") },
        { .ptr = &(uint32_t){ 42 }, .size = sizeof(uint32_t) }
    };

    mdv_data const *rows[] = { row };

    mu_check(mdv_rowset_append(insert_rowset, 0, rows, 1) == 1);
    mu_check(mdv_insert(client, insert_rowset) == MDV_OK);
    mdv_rowset_release(insert_rowset);

    // Read
    mdv_rowset *select_rowset = mdv_select(client, table, 0, "");
    mu_check(select_rowset);

    mdv_enumerator *enumerator = mdv_rowset_enumerator(select_rowset);
    mu_check(enumerator);

    mu_check(mdv_enumerator_next(enumerator) == MDV_OK);

    mdv_objid row_id = *mdv_enumerator_row_id(enumerator);

    mdv_enumerator_release(enumerator);
    mdv_rowset_release(select_rowset);

    // Update
    mdv_rowset *update_rowset = mdv_rowset_create(table);
    mu_check(update_rowset);

    mdv_data update_row[] =
    {
        { .ptr = "Jane Doe", .size = sizeof("Jane Doe") },
        { .ptr = &(uint32_t){ 43 }, .size = sizeof(uint32_t) }
    };

    mdv_data const *update_rows[] = { update_row };

    mu_check(mdv_rowset_append(update_rowset, 0, update_rows, 1) == 1);
    mu_check(mdv_update(client, table, &row_id, update_rowset) == MDV_OK);
    mdv_rowset_release(update_rowset);

    // Delete
    mu_check(mdv_delete(client, table, &row_id) == MDV_OK);

    mdv_table_release(table);
    mdv_client_close(client);
}


MU_TEST_SUITE(crud)
{
    MU_RUN_TEST(crud, create_read_update_delete);
}