#!/bin/bash
set -e

# Clean previous build artifacts
rm -rf build-output
mkdir -p build-output

# Build image with cache optimization
docker build -t medveddb-test:1 -f Dockerfile.dev .

# Run build process in clean container environment
docker run --rm -it --name medveddb-builder \
  -v "$(pwd)":/app \  # Map the current directory to /app in the container
  -v mdv_build_cache:/app/build \  # Mount the build cache volume to /app/build
  -w /app \  # Set the working directory to /app
  medveddb-test \
  /bin/bash -c "
    set -e
    rm -rf build
    mkdir build
    cd build
    cmake .. -DBUILD_JNI=ON -DCMAKE_BUILD_TYPE=Release
    make --jobs=\$(nproc)
    echo 'Core components built successfully'
  "

echo "MedvedDB core components built successfully in Docker container"