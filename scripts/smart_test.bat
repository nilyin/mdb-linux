@echo off
REM Smart test runner - DEPRECATED, use specific test scripts

echo === MedvedDB Smart Test Runner ===
echo ⚠️  DEPRECATED: This script is maintained for compatibility
echo ✅ RECOMMENDED: Use specific test scripts for better performance
echo.

set ACTION=%1
if "%ACTION%"=="" set ACTION=help

if "%ACTION%"=="build" (
    echo Running incremental build...
    call scripts\incremental_build.bat
) else if "%ACTION%"=="test" (
    echo Running core tests...
    call scripts\test_data_consistency.bat
) else if "%ACTION%"=="full" (
    echo Running complete test suite...
    call scripts\run_complete_tests.bat
) else if "%ACTION%"=="server" (
    echo Running server verification...
    call scripts\run_core_tests.bat
) else if "%ACTION%"=="clean" (
    echo Cleaning build cache...
    docker volume rm mdv_build_cache 2>nul
    echo Build cache cleared
) else (
    echo Usage: %0 [build^|test^|full^|server^|clean^|help]
    echo.
    echo RECOMMENDED SCRIPTS:
    echo   test_data_consistency.bat    - Core components test
    echo   run_core_tests.bat          - Core + server verification
    echo   run_complete_tests.bat      - Complete test suite
    echo   run_all_tests_final.bat     - Comprehensive execution
    echo   incremental_build.bat       - Build only
    echo.
    echo LEGACY OPTIONS:
    echo   build  - Incremental build (use incremental_build.bat)
    echo   test   - Core tests (use test_data_consistency.bat)
    echo   full   - Complete tests (use run_complete_tests.bat)
    echo   server - Server tests (use run_core_tests.bat)
    echo   clean  - Clean build cache
)