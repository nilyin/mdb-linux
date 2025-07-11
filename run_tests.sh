#!/bin/bash

echo "=== MedvedDB Test Execution Plan ==="
echo "Starting comprehensive C language test suite..."
echo

# Build the project
echo "Step 1: Building project..."
mkdir -p build
cd build
cmake ..
if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

echo "Build completed successfully"
echo

# Setup server configuration and data directory
echo "Step 2: Configuring MedvedDB server..."
mkdir -p data
cp ../assets/conf/medved.conf ./medved.conf
echo "Server configuration ready"
echo

# Start MedvedDB server
echo "Step 3: Starting MedvedDB server..."
./mdv_service/mdv_service -c ./medved.conf &
SERVER_PID=$!
echo "Server started with PID: $SERVER_PID"

# Wait for server to be ready
echo "Waiting for server to start..."
sleep 3

# Check if server is running
if ! kill -0 $SERVER_PID 2>/dev/null; then
    echo "ERROR: Server failed to start"
    exit 1
fi

# Test server connectivity
echo "Testing server connectivity..."
for i in {1..10}; do
    if nc -z localhost 4800 2>/dev/null; then
        echo "Server is ready on port 4800"
        break
    fi
    if [ $i -eq 10 ]; then
        echo "ERROR: Server not responding after 10 attempts"
        kill $SERVER_PID 2>/dev/null
        exit 1
    fi
    sleep 1
done
echo

# Run tests with detailed monitoring
echo "Step 4: Executing test suites..."
echo "Expected test execution order:"
echo "  1. Platform Suite (26 tests)"
echo "  2. Types Suite (4 tests)" 
echo "  3. Crypto Suite (1 test)"
echo "  4. Storage Suite (6 tests)"
echo "  5. CRUD Suite (1 test)"
echo "Total expected: 38 tests"
echo

echo "=== TEST EXECUTION START ==="
./mdv_tests/mdv_tests
TEST_EXIT_CODE=$?
echo "=== TEST EXECUTION END ==="
echo

# Cleanup: Stop server
echo "Step 5: Stopping server..."
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null
echo "Server stopped"
echo

# Analyze results
if [ $TEST_EXIT_CODE -eq 0 ]; then
    echo "✅ ALL TESTS PASSED"
else
    echo "❌ TESTS FAILED (Exit code: $TEST_EXIT_CODE)"
fi

echo
echo "Test execution completed."