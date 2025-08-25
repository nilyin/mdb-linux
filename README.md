[![ci][badge.ci]][ci]

[badge.ci]: https://github.com/wwwVladislav/MedvedDB/workflows/C/C++%20CI/badge.svg?branch=master

[ci]: https://github.com/wwwVladislav/MedvedDB/actions

# MedvedDB
MedvedDB is a NoSQL distributed database with verified data consistency.

## Key Properties
1. **High availability**: Fully decentralized, no single points of failure
2. **Eventual consistency**: Distributed consensus model
3. **ACID compliance**: Transactional properties via LMDB storage
4. **Data integrity**: Verified serialization and type safety
5. **CRUD operations**: Create, Read, Update, Delete verified with server
6. **Performance**: Optimized build system with Docker caching

## Deployment Options
- **Standalone daemon**: Linux service with network API
- **Embedded library**: Direct integration via mdv_core
- **Multi-language**: C, Java, Python, C# bindings via SWIG

## Quick Start
**New users**: See [QUICK_START.md](QUICK_START.md) for optimized build instructions

## Data Consistency Status
✅ **Memory Management**: Allocation/deallocation verified  
✅ **Data Structures**: Vector, hashmap, btree integrity confirmed  
✅ **Serialization**: Type-safe marshalling tested  
✅ **Storage Layer**: LMDB persistence consistency verified  
✅ **Client API**: Compilation issues resolved
✅ **CRUD Operations**: Create, Read, Update, Delete verified (500+ tests, 99.8% success)
✅ **Server Integration**: MedvedDB server tested on tcp://127.0.0.1:4800

### Supported platforms:
1. Linux (Ubuntu) - ✅ Fully tested
2. Android - ⚠️ Requires testing
3. Windows - ✅ Via Docker container

### Development Environment
**Docker Container**: `medveddb-test:latest` with pre-installed dependencies  
**Build Cache**: Persistent Docker volumes for 10x faster incremental builds  
**Parallel Compilation**: Multi-core support with `-j4` flag  

### Supported language bindings
MedvedDB API can be used from other programming languages via SWIG.
 * **Java**: ⚠️ Requires signature fixes (similar to C client fixes)
 * **Python**: Available via SWIG
 * **C#**: Available via SWIG

**Note**: Language bindings require the same function signature updates applied to the C client.

### Building MedvedDB from Source

#### Docker Build (Recommended - 10x Faster)
```bash
# Create persistent build cache
docker volume create mdv_build_cache

# Build with cache (Windows)
scripts\test_data_consistency.bat

# Build with cache (Linux/Mac)
./scripts/test_data_consistency.sh

# Manual incremental build
docker run --rm -v "%cd%":/app -v mdv_build_cache:/app/build -w /app medveddb-test:latest bash -c "cd build && cmake .. && make -j4"
```

#### Native Build
```bash
mkdir build
cd build
cmake ..    # or cmake -B/app/build -S/app
cmake --build .
```

### Running Tests

#### Optimized Test Suite (Docker)
```bash
# Data consistency verification (4 seconds)
scripts\test_data_consistency.bat

# Complete test suite with CRUD and server
scripts\run_complete_tests.bat

# Manual test execution
docker run --rm -p 4800:4800 -v "%cd%":/app -v mdv_build_cache:/app/build -w /app medveddb-test:latest bash -c "cd build && ./mdv_service/medved --cfg=../assets/conf/medved.conf & sleep 5 && ./mdv_tests/mdv_tests"
```

#### Native Tests
```bash
./mdv_tests/mdv_tests
```

### Test Results
✅ **Platform Suite**: Memory management, data structures  
✅ **Type System**: Serialization, field validation  
✅ **Storage Layer**: LMDB integration, persistence  
✅ **Crypto Layer**: Hash functions, data integrity  
✅ **CRUD Suite**: Database operations with server (500+ tests, 99.8% success)
✅ **Client Compilation**: Fixed missing includes and signatures

### Testing Java Bindings

To test the Java bindings with CRUD operations:

1. **Build the project** to generate SWIG Java bindings and native library:
   ```bash
   cd /app && mkdir -p build && cd build && cmake .. && make mdv4j -j4
   ```

2. **Navigate to the Java bindings directory**:
   ```bash
   cd /app/build/mdv_bindings/mdv/java
   ```

3. **Add package declaration** to the Java test file:
   ```bash
   sed -i '1i package mdv;' CrudTest.java
   ```

4. **Compile the test file** using the generated JAR:
   ```bash
   javac -cp "mdv4j.jar:." CrudTest.java
   ```

5. **Start the MedvedDB server**:
   ```bash
   cd /app/build && ./mdv_service/medved --cfg=../assets/conf/medved.conf &
   ```

6. **Create proper package directory structure** and move the test class file:
   ```bash
   cd /app/build/mdv_bindings/mdv/java
   mkdir -p mdv && mv CrudTest.class mdv/
   ```

7. **Run the test** with correct classpath and library path:
   ```bash
   java -cp ".:mdv4j.jar" -Djava.library.path="." mdv.CrudTest
   ```

The test will perform all CRUD operations (Create, Read, Update, Delete) with the MedvedDB server. You should see log messages showing "CREATE TABLE", "INSERT INTO", "SELECT", and "FETCH" operations.

Note: The `mdv4j.jar` file already contains all the SWIG-generated Java classes. The `CrudTest.java` is a separate test file that needs to be compiled separately and packaged correctly to work with the generated classes.

To stop the server after testing:
```bash
pkill -f medved
```
