#!/bin/bash
set -e

# Clean previous build artifacts
rm -rf build-output
mkdir -p build-output

# Build image with cache optimization
docker build -t medveddb-test -f Dockerfile.dev .

# Run build process in clean container environment
docker run --rm -it --name medveddb-builder \
  -v $(pwd):/workspace \
  -w /workspace \
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