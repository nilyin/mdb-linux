#include "mdv_perf.h"
#include <stdio.h>

int main(void) {
    printf("Running MedvedDB Performance Tests...\n");
    mdv_run_performance_tests();
    return 0;
}