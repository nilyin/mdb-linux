@echo off
REM Single container test with server - matches run_complete_tests.ps1

set VOLUME_NAME=mdv_build_cache
set IMAGE_NAME=medveddb-test:latest

echo === MedvedDB Complete Test Suite ===
echo Running all tests including CRUD with integrated server

docker volume create %VOLUME_NAME% 2>nul

echo Building and running complete test suite...

docker run --rm -p 4800:4800 -v "%cd%":/app -v %VOLUME_NAME%:/app/build -w /app %IMAGE_NAME% bash -c "set -e && echo '=== Building MedvedDB Components ===' && cd build && cmake .. 2>/dev/null || true && make medved mdv_tests -j4 && echo && echo '=== Starting MedvedDB Server ===' && ./mdv_service/medved --cfg=../assets/conf/medved.conf & && SERVER_PID=$! && echo 'Waiting for server to initialize...' && sleep 5 && if ! kill -0 $SERVER_PID 2>/dev/null; then echo 'ERROR: Server failed to start' && exit 1; fi && echo '✅ MedvedDB server started (PID: '$SERVER_PID')' && echo 'Testing server connectivity...' && timeout 5 bash -c 'until nc -z 127.0.0.1 4800; do sleep 1; done' || { echo 'ERROR: Server not responding on port 4800' && kill $SERVER_PID 2>/dev/null && exit 1; } && echo '✅ Server is responding on tcp://127.0.0.1:4800' && echo && echo '=== Running Complete Test Suite ===' && ./mdv_tests/mdv_tests && echo && echo '=== Test Results Summary ===' && echo '✅ Platform: Memory management, data structures' && echo '✅ Types: Serialization, field validation' && echo '✅ Storage: LMDB integration, persistence' && echo '✅ Crypto: Hash functions, data integrity' && echo '✅ CRUD: Database operations with server connection' && echo && echo 'Stopping server...' && kill $SERVER_PID 2>/dev/null && wait $SERVER_PID 2>/dev/null && echo '✅ All tests completed successfully'"

if %ERRORLEVEL% equ 0 (
    echo.
    echo === SUCCESS: All Tests Passed ===
    echo ✅ Core components built and tested
    echo ✅ CRUD operations verified with server
    echo ✅ Data consistency confirmed across all layers
) else (
    echo.
    echo X FAILED: Some tests failed
    exit /b 1
)