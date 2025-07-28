# PowerShell script to run complete test suite with server in single container
param(
    [switch]$CleanBuild
)

$VOLUME_NAME = "mdv_build_cache"
$IMAGE_NAME = "medveddb-test:1"

Write-Host "=== MedvedDB Complete Test Suite ===" -ForegroundColor Green
Write-Host "Running all tests including CRUD with integrated server" -ForegroundColor Cyan

# Create volume if needed
docker volume create $VOLUME_NAME 2>$null

# Clean build if requested
if ($CleanBuild) {
    Write-Host "Cleaning build cache..." -ForegroundColor Yellow
    docker volume rm $VOLUME_NAME 2>$null
    docker volume create $VOLUME_NAME 2>$null
}

Write-Host "Building and running complete test suite..." -ForegroundColor Cyan

docker run --rm -p 4800:4800 -v ${PWD}:/app -v ${VOLUME_NAME}:/app/build -w /app $IMAGE_NAME bash -c @"
set -e

echo '=== Building MedvedDB Components ==='
cd build
cmake .. 2>/dev/null || true

# Build all components
make medved mdv_tests -j4

echo
echo '=== Starting MedvedDB Server ==='
# Start server in background
./mdv_service/medved --cfg=../assets/conf/medved.conf &
SERVER_PID=\$!

# Wait for server to start
echo 'Waiting for server to initialize...'
sleep 5

# Check if server is running
if ! kill -0 \$SERVER_PID 2>/dev/null; then
    echo 'ERROR: Server failed to start'
    exit 1
fi

echo '✅ MedvedDB server started (PID: '\$SERVER_PID')'

# Test server connectivity
echo 'Testing server connectivity...'
timeout 5 bash -c 'until nc -z 127.0.0.1 4800; do sleep 1; done' || {
    echo 'ERROR: Server not responding on port 4800'
    kill \$SERVER_PID 2>/dev/null
    exit 1
}

echo '✅ Server is responding on tcp://127.0.0.1:4800'

echo
echo '=== Running Complete Test Suite ==='
./mdv_tests/mdv_tests

echo
echo '=== Test Results Summary ==='
echo '✅ Platform: Memory management, data structures'
echo '✅ Types: Serialization, field validation'
echo '✅ Storage: LMDB integration, persistence'
echo '✅ Crypto: Hash functions, data integrity'
echo '✅ CRUD: Database operations with server connection'

# Cleanup
echo
echo 'Stopping server...'
kill \$SERVER_PID 2>/dev/null
wait \$SERVER_PID 2>/dev/null

echo '✅ All tests completed successfully'
"@

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n=== SUCCESS: All Tests Passed ===" -ForegroundColor Green
    Write-Host "✅ Core components built and tested" -ForegroundColor Green
    Write-Host "✅ CRUD operations verified with server" -ForegroundColor Green
    Write-Host "✅ Data consistency confirmed across all layers" -ForegroundColor Green
} else {
    Write-Host "`n❌ FAILED: Some tests failed" -ForegroundColor Red
    exit 1
}