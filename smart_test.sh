#!/bin/bash
# Smart test runner that minimizes rebuilds

set -e

CONTAINER_NAME="mdv_dev_container"
VOLUME_NAME="mdv_build_cache"

# Function to run command in persistent container
run_in_container() {
    # Check if container exists and is running
    if ! docker ps -q -f name=$CONTAINER_NAME | grep -q .; then
        echo "Starting development container..."
        
        # Create volume if it doesn't exist
        docker volume create $VOLUME_NAME 2>/dev/null || true
        
        # Start container in background
        docker run -d \
            --name $CONTAINER_NAME \
            -v "$(pwd)":/app \
            -v $VOLUME_NAME:/app/build \
            -w /app \
            medveddb-test:latest \
            tail -f /dev/null
        
        echo "Container started: $CONTAINER_NAME"
    fi
    
    # Execute command in running container
    docker exec $CONTAINER_NAME bash -c "$1"
}

# Function to stop and remove container
cleanup() {
    echo "Cleaning up container..."
    docker stop $CONTAINER_NAME 2>/dev/null || true
    docker rm $CONTAINER_NAME 2>/dev/null || true
}

# Trap to cleanup on exit
trap cleanup EXIT

case "${1:-test}" in
    "build")
        echo "=== Incremental Build ==="
        run_in_container "
            if [ ! -f build/CMakeCache.txt ]; then
                echo 'Initial build...'
                mkdir -p build && cd build && cmake ..
            fi
            cd build && make mdv_tests -j\$(nproc)
        "
        ;;
    
    "test")
        echo "=== Running Tests ==="
        run_in_container "
            cd build 2>/dev/null || (mkdir -p build && cd build && cmake ..)
            make mdv_tests -j\$(nproc) && ./mdv_tests/mdv_tests
        "
        ;;
    
    "client")
        echo "=== Building Client ==="
        run_in_container "
            cd build 2>/dev/null || (mkdir -p build && cd build && cmake ..)
            make mdv -j\$(nproc)
        "
        ;;
    
    "clean")
        echo "=== Cleaning Build Cache ==="
        docker volume rm $VOLUME_NAME 2>/dev/null || true
        echo "Build cache cleared"
        ;;
    
    "shell")
        echo "=== Opening Shell ==="
        run_in_container "bash"
        ;;
    
    *)
        echo "Usage: $0 [build|test|client|clean|shell]"
        echo "  build  - Incremental build"
        echo "  test   - Build and run tests"
        echo "  client - Build client"
        echo "  clean  - Clean build cache"
        echo "  shell  - Open shell in container"
        ;;
esac