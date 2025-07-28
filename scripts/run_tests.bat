@echo off
REM Legacy test runner - now uses Docker for consistency
REM DEPRECATED: Use run_complete_tests.bat instead

set VOLUME_NAME=mdv_build_cache
set IMAGE_NAME=medveddb-test:1

echo === MedvedDB Legacy Test Runner ===
echo ⚠️  DEPRECATED: This script is maintained for compatibility
echo ✅ RECOMMENDED: Use run_complete_tests.bat for better performance
echo.

REM Create volume if needed
docker volume create %VOLUME_NAME% 2>nul

echo Running tests in Docker container...
docker run --rm -p 4800:4800 ^
  -v "%cd%":/app ^
  -v %VOLUME_NAME%:/app/build ^
  -w /app ^
  %IMAGE_NAME% ^
  bash -c "set -e && echo '=== Building MedvedDB Components ===' && cd build && cmake .. 2>/dev/null || true && make medved mdv_tests -j4 && echo && echo '=== Starting MedvedDB Server ===' && ./mdv_service/medved --cfg=../assets/conf/medved.conf & && SERVER_PID=$! && echo 'Waiting for server to start...' && sleep 5 && if ! kill -0 $SERVER_PID 2>/dev/null; then echo 'ERROR: Server failed to start' && exit 1; fi && echo 'Testing server connectivity...' && timeout 10 bash -c 'until nc -z 127.0.0.1 4800; do sleep 1; done' || { echo 'ERROR: Server not responding' && kill $SERVER_PID 2>/dev/null && exit 1; } && echo '✅ Server is ready on port 4800' && echo && echo '=== Running Test Suites ===' && echo 'Expected: Platform, Types, Crypto, Storage, CRUD' && echo && ./mdv_tests/mdv_tests && TEST_EXIT_CODE=$? && echo && echo 'Stopping server...' && kill $SERVER_PID 2>/dev/null && wait $SERVER_PID 2>/dev/null && if [ $TEST_EXIT_CODE -eq 0 ]; then echo '✅ ALL TESTS PASSED'; else echo '❌ TESTS FAILED' && exit 1; fi"

if %ERRORLEVEL% equ 0 (
    echo.
    echo ✅ Legacy test execution completed
    echo 💡 For better performance, use: run_complete_tests.bat
) else (
    echo.
    echo ❌ Legacy test execution failed
    exit /b 1
)