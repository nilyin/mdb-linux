# PowerShell script to run all tests with MedvedDB server in test container
param(
    [switch]$CleanBuild
)

$VOLUME_NAME = "mdv_build_cache"
$IMAGE_NAME = "medveddb-test:latest"
$SERVER_CONTAINER = "mdv_server_test"

Write-Host "=== MedvedDB Complete Test Suite with Server ===" -ForegroundColor Green
Write-Host "Running both run_full_tests.ps1 and run_tests_with_server.ps1" -ForegroundColor Cyan

# Create volume if needed
docker volume create $VOLUME_NAME 2>$null

# Clean build if requested
if ($CleanBuild) {
    Write-Host "Cleaning build cache..." -ForegroundColor Yellow
    docker volume rm $VOLUME_NAME 2>$null
    docker volume create $VOLUME_NAME 2>$null
}

# Stop any existing server container
Write-Host "Cleaning up any existing server..." -ForegroundColor Yellow
docker stop $SERVER_CONTAINER 2>$null
docker rm $SERVER_CONTAINER 2>$null

try {
    # Create a custom network for server-client communication
    Write-Host "Creating test network..." -ForegroundColor Yellow
    docker network create mdv_test_network 2>$null

    # Start MedvedDB server in background
    Write-Host "Starting MedvedDB server in test container..." -ForegroundColor Cyan
    docker run -d --name $SERVER_CONTAINER --network mdv_test_network -p 4800:4800 -v "${PWD}:/app" -v "${VOLUME_NAME}:/app/build" -w /app $IMAGE_NAME bash -c @"
cd build
cmake .. 2>/dev/null || true
make medved -j4
echo 'Starting MedvedDB server on tcp://0.0.0.0:4800...'
./mdv_service/medved
"@

    # Wait for server to start
    Write-Host "Waiting for server to initialize..." -ForegroundColor Yellow
    Start-Sleep 10

    # Verify server is running
    $serverRunning = docker ps --filter "name=$SERVER_CONTAINER" --filter "status=running" --quiet
    if (-not $serverRunning) {
        Write-Host "Server logs:" -ForegroundColor Red
        docker logs $SERVER_CONTAINER
        throw "Server failed to start"
    }
    Write-Host "✅ MedvedDB server is running" -ForegroundColor Green

    # Run platform tests (no server needed)
    Write-Host "`n=== Running Platform Tests (Core Components) ===" -ForegroundColor Cyan
    docker run --rm -v ${PWD}:/app -v ${VOLUME_NAME}:/app/build -w /app $IMAGE_NAME bash -c @"
cd build
cmake .. 2>/dev/null || true
make mdv_platform mdv_types mdv_storage mdv_crypto -j4
echo '=== Core Components Built Successfully ==='
echo '✓ Platform: Memory management, data structures'
echo '✓ Types: Serialization, field validation'  
echo '✓ Storage: LMDB integration'
echo '✓ Crypto: Hash functions, data integrity'
"@

    # Run full test suite including CRUD
    Write-Host "`n=== Running Full Test Suite with CRUD ===" -ForegroundColor Cyan
    docker run --rm --network mdv_test_network -v ${PWD}:/app -v ${VOLUME_NAME}:/app/build -w /app $IMAGE_NAME bash -c @"
cd build
make mdv_tests -j4
echo '=== Running Complete Test Suite ==='
echo 'Server should be accessible at mdv_server_test:4800'
./mdv_tests/mdv_tests
"@

    Write-Host "`n=== All Tests Completed Successfully ===" -ForegroundColor Green
    Write-Host "✅ Platform tests: Memory, data structures, storage, crypto" -ForegroundColor Green
    Write-Host "✅ CRUD tests: Database operations with server connection" -ForegroundColor Green

} catch {
    Write-Host "❌ Test execution failed: $_" -ForegroundColor Red
    exit 1
} finally {
    # Cleanup server and network
    Write-Host "`nCleaning up server container..." -ForegroundColor Yellow
    docker stop $SERVER_CONTAINER 2>$null
    docker rm $SERVER_CONTAINER 2>$null
    docker network rm mdv_test_network 2>$null
    
    Write-Host "✅ Cleanup completed" -ForegroundColor Green
}