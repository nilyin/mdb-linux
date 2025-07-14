# SWIG Debugging and Fixes

## Summary of Findings

1.  **C-Level Segmentation Fault:** The test suite is failing with a segmentation fault, which appears to be caused by a pre-existing issue in the C-level code and is not related to the Java bindings.
2.  **Java Iterator Issue:** The `RowSetEnumerator` iterator is not functioning correctly due to a flaw in the `hasNext()` and `next()` methods, which causes the iterator to skip elements.
3.  **Build System Issues:** The build system has a number of issues, including:
    *   The `run_complete_tests.sh` script does not properly stop running Docker containers, which can lead to "port is already allocated" errors.
    *   The `mdv_platform/CMakeLists.txt` file uses a `GLOB_RECURSE` command, which can make it difficult to identify the source of build issues.
    *   The test suite headers do not consistently include the `minunit.h` header, which can lead to compilation errors.

## Proposed Next Steps

1.  **Isolate the Segmentation Fault:**
    *   Continue to isolate the segmentation fault by selectively disabling tests in the `mdv_tests/mdv_platform.h` file.
    *   Once the problematic test is identified, use `gdb` to get a backtrace of the crash and identify the exact line of code that is causing the issue.
2.  **Fix the Java Iterator:**
    *   Re-implement the fix for the `RowSetEnumerator` iterator in `assets/swig/mdv/mdv_rowset.i`.
    *   Add a `peek` field to the `mdv_rows_enumerator` struct to cache the next element.
    *   Update the `hasNext()` and `next()` methods to use the new pre-fetching logic.
3.  **Improve the Build System:**
    *   Modify the `run_complete_tests.sh` script to ensure that all running Docker containers are stopped before starting a new test run.
    *   Modify the `mdv_platform/CMakeLists.txt` file to explicitly list the source files.
    *   Add the `#include "minunit.h"` directive to all test suite headers.