#!/bin/bash
# Create persistent build cache using Docker volumes

# Create named volume for build cache if it doesn't exist
docker volume create mdv_build_cache 2>/dev/null

# Run with persistent build directory
docker run --rm \
  -v "$(pwd)":/app \
  -v mdv_build_cache:/app/build \
  -w /app \
  medveddb-test:1 \
  bash -c "$*"

echo
echo "Build cache is preserved in Docker volume 'mdv_build_cache'"
echo "To clean cache: docker volume rm mdv_build_cache"