@echo off
REM Create persistent build cache using Docker volumes

REM Create named volume for build cache if it doesn't exist
docker volume create mdv_build_cache 2>nul

REM Run with persistent build directory
docker run --rm ^
  -v "%cd%":/app ^
  -v mdv_build_cache:/app/build ^
  -w /app ^
  medveddb-test:latest ^
  bash -c "%*"

echo.
echo Build cache is preserved in Docker volume 'mdv_build_cache'
echo To clean cache: docker volume rm mdv_build_cache