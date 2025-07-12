# Java Iterator Tests

## Files
- `java_iterator_test.java` - Complete iterator test suite (10 test cases)
- `simple_iterator_test.java` - Basic functionality verification
- `ITERATOR_TEST_RESULTS.md` - Detailed test execution results

## Usage
```bash
# Build Java bindings
docker run --rm -v "$(pwd)":/app -v mdv_build_cache:/app/build -w /app/build medveddb-test:latest bash -c "cmake .. && make mdv4j -j4"

# Run tests
cd mdv_bindings/mdv/java/tests
javac -cp /path/to/build/mdv_bindings/mdv/java:. java_iterator_test.java
java -Djava.library.path=/path/to/build -cp /path/to/build/mdv_bindings/mdv/java:. java_iterator_test
```

## Test Coverage
- Try-with-resources pattern
- Manual cleanup
- Enhanced for-each loop
- Backward compatibility
- Edge cases and exception handling

## Status
❌ Blocked by SWIG compilation errors - see ITERATOR_TEST_RESULTS.md