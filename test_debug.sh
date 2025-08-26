#!/bin/bash

cd /app/build

# Start server in background
./mdv_service/medved --cfg=../assets/conf/medved.conf &
SERVER_PID=$!

# Wait for server to start
sleep 5

# Run performance test with timeout
timeout 30 ./mdv_tests/mdv_perf 2>&1 | head -100

# Kill server
kill $SERVER_PID 2>/dev/null

echo "Test completed"