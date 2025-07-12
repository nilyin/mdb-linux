#!/bin/bash
# Incremental build script that only rebuilds what's necessary

set -e

echo "=== MedvedDB Incremental Build ==="

# Check if build directory exists and has CMakeCache
if [ ! -f "build/CMakeCache.txt" ]; then
    echo "No existing build found, performing initial build..."
    rm -rf build
    mkdir -p build
    cd build
    cmake ..
    echo "CMake configuration complete"
else
    echo "Existing build found, using incremental build..."
    cd build
fi

# Build only what's needed
echo "Building changed components..."

# Build core libraries first (these change less frequently)
echo "Building platform libraries..."
make mdv_platform -j$(nproc) 2>/dev/null || echo "Platform already up to date"

echo "Building type libraries..."
make mdv_types -j$(nproc) 2>/dev/null || echo "Types already up to date"

echo "Building network libraries..."
make mdv_net -j$(nproc) 2>/dev/null || echo "Network already up to date"

echo "Building API libraries..."
make mdv_api -j$(nproc) 2>/dev/null || echo "API already up to date"

# Build tests
echo "Building tests..."
make mdv_tests -j$(nproc)

echo "=== Build Complete ==="
echo "To run tests: ./mdv_tests/mdv_tests"
echo "To run client: make mdv && ./mdv_client/mdv"