#pragma once
#include <minunit.h>

// Forward declaration
void create_read_update_delete(void);

MU_TEST_SUITE(crud)
{
    MU_RUN_TEST(create_read_update_delete);
}