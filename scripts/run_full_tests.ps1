# PowerShell script to run full tests including CRUD with incremental build
param(
    [switch]$StartServer,
    [switch]$StopServer
)

$VOLUME_NAME = "mdv_build_cache"
$IMAGE_NAME = "medveddb-test:1"
$SERVER_CONTAINER = "mdv_server"

Write-Host "=== MedvedDB Full Test Suite (PowerShell) ===" -ForegroundColor Green
Write-Host "Docker volume location: /var/lib/docker/volumes/$VOLUME_NAME/_data" -ForegroundColor Yellow

# Create volume if needed
docker volume create $VOLUME_NAME 2>$null

if ($StartServer) {
    Write-Host "Starting MedvedDB server..." -ForegroundColor Cyan
    docker run -d --name $SERVER_CONTAINER -p 4800:4800 -v ${PWD}:/app -v ${VOLUME_NAME}:/app/build -w /app $IMAGE_NAME bash -c "cd build && make medved -j4 && ./mdv_service/medved"
    Start-Sleep 5
}

Write-Host "Building all components with cache..." -ForegroundColor Cyan
docker run --rm -v ${PWD}:/app -v ${VOLUME_NAME}:/app/build -w /app $IMAGE_NAME bash -c @"
cd build
cmake .. 2>/dev/null || true
make mdv_tests -j4
echo '=== Running Full Test Suite ==='
./mdv_tests/mdv_tests
"@

if ($StopServer) {
    Write-Host "Stopping server..." -ForegroundColor Cyan
    docker stop $SERVER_CONTAINER 2>$null
    docker rm $SERVER_CONTAINER 2>$null
}

Write-Host "=== Test Complete ===" -ForegroundColor Green