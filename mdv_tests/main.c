#include "mdv_platform.h"
#include "mdv_types.h"
#include "mdv_core.h"
#include "mdv_crypto.h"
#include "mdv_storage.h"
#include "mdv_crud.h"
#include "mdv_perf.h"
#include <minunit.h>
#include <mdv_log.h>
#include <string.h>


int main(int argc, char *argv[])
{
    mdv_logf_set_level(ZF_LOG_WARN);

    // Check if performance test requested
    if (argc > 1 && strcmp(argv[1], "--perf") == 0) {
        printf("Running MedvedDB Performance Tests...\n");
        mdv_run_performance_tests();
        return 0;
    }

    // Run standard test suites
    MU_RUN_SUITE(platform);
    MU_RUN_SUITE(types);
    MU_RUN_SUITE(crypto);
    MU_RUN_SUITE(storage);
    MU_RUN_SUITE(crud);
    MU_REPORT();

    return minunit_status;
}

