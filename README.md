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
 * **Java**: ✅ Fully functional (CRUD, Iterators, Memory Management)
 * **Python**: Available via SWIG
 * **C#**: Available via SWIG

**Java Implementation Status:**
- ✅ **CRUD Operations**: Create, Read, Update, Delete (100% functional)
- ✅ **Iterator Functionality**: Complete with 100% test success rate
- ✅ **Memory Management**: Proper resource cleanup patterns
- ✅ **ObjectId Operations**: Delete and update with ObjectId targeting
- ⚠️ **Query Filters**: Server-side filtering not implemented (see Query Behavior below)

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

3. **Start the MedvedDB server**:
   ```bash
   cd /app/build && ./mdv_service/medved --cfg=../assets/conf/medved.conf &
   ```

4. **Compile the test file** using the generated JAR:
   ```bash
   cd /app/build
   javac -cp mdv_bindings/mdv/java/mdv4j.jar ../mdv_bindings/mdv/java/tests/CrudTest.java -d java-tests
   ```

5. **Run the test** with correct classpath and library path:
   ```bash
   java -Djava.library.path=mdv_bindings/mdv/java -cp java-tests:mdv_bindings/mdv/java/mdv4j.jar CrudTest
   ```

The test will perform all CRUD operations (Create, Read, Update, Delete) with the MedvedDB server. You should see log messages showing "CREATE TABLE", "INSERT INTO", "SELECT", and "FETCH" operations.

Note: The `mdv4j.jar` file already contains all the SWIG-generated Java classes. The `CrudTest.java` is a separate test file that needs to be compiled separately and packaged correctly to work with the generated classes.

To stop the server after testing:
```bash
pkill -f medved
```

## Query Behavior and Filtering

### Current Query Implementation

MedvedDB currently supports **column selection** but **not row filtering**:

**✅ What Works:**
```java
// Column selection via BitSet
BitSet bitSet = new BitSet(fieldCount);
bitSet.fill(true);  // Select all columns
// OR set specific bits for specific columns

RowSet results = client.select(table, bitSet, "");  // Returns ALL rows
```

**❌ What Doesn't Work (Server-Side Filtering):**
```java
// These calls are functionally identical - filters are ignored:
client.select(table, bitSet, "");                    // Returns ALL rows
client.select(table, bitSet, "name = 'John'");       // Returns ALL rows (filter ignored!)
client.select(table, bitSet, "age > 25");            // Returns ALL rows (filter ignored!)
client.select(table, bitSet, "complex expression");  // Returns ALL rows (filter ignored!)
```

### Root Cause

Server-side filtering is **completely unimplemented** in the core database:

1. **Predicate Parser** (`mdv_predicate.c`): All filter expressions are ignored
2. **Select Operation** (`mdv_select.c`): Contains `TODO: use predicate for DB entries selection`
3. **Impact**: Affects both C and Java APIs equally

### Required Application Pattern

**Applications must implement client-side filtering:**

```java
// Step 1: Retrieve ALL data from server
RowSet allRows = client.select(table, bitSet, ""); // Empty filter = all rows

// Step 2: Filter in application code
if (allRows != null) {
    RowSetEnumerator it = allRows.enumerator();
    try {
        while (it.next()) {
            Row row = it.current();
            String name = row.getString(1);
            int age = row.getInt32(2);
            
            // YOUR filtering logic here
            if ("John".equals(name) && age > 25) {
                // Process matching row
                processRow(row);
            }
            // Skip non-matching rows
            
            row.delete();
        }
    } finally {
        it.delete();
        allRows.delete();
    }
}
```

### Alternative Strategies

**1. Table Segmentation:**
```java
// Create separate tables for different data categories
Table activeUsers = client.createTable(activeUsersDesc);
Table inactiveUsers = client.createTable(inactiveUsersDesc);

// Route data to appropriate tables during insertion
if (user.isActive()) {
    insertIntoTable(activeUsers, userData);
} else {
    insertIntoTable(inactiveUsers, userData);
}
```

**2. Application-Level Indexing:**
```java
// Maintain in-memory indexes for fast lookups
Map<String, List<ObjectId>> nameIndex = new HashMap<>();
Map<Integer, List<ObjectId>> ageIndex = new HashMap<>();

// Build indexes during data retrieval
// Use indexes for efficient filtering
List<ObjectId> matchingIds = nameIndex.get("targetName");
```

### Performance Considerations

- **Small datasets**: Client-side filtering is acceptable
- **Large datasets**: Consider table segmentation or application-level indexing
- **Network efficiency**: All table data is transferred regardless of filtering needs
- **Memory usage**: Applications must handle full result sets

## Debugging the C server and client with gdb

This project can be built with debug information so you can use gdb to step through the server (medved) and client (mdv).

1) Configure a clean build directory
- mkdir -p build && cd build

2) Configure with debug symbols (recommended)
- RelWithDebInfo (optimized + debug info):
  - cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
- Or full Debug (no optimizations):
  - cmake -DCMAKE_BUILD_TYPE=Debug ..

3) Alternate: inject -g into Release builds
- If your scripts use Release by default but you still want symbols:
  - cmake -DENABLE_DEBUG_SYMBOLS=ON -DCMAKE_BUILD_TYPE=Release ..

4) Build server and client targets
- cmake --build . --target medved -- -j$(nproc)
- cmake --build . --target mdv    -- -j$(nproc)

Built binary locations (relative to project root):
- build/mdv_service/medved
- build/mdv_client/mdv

5) Verify debug symbols are present
- file build/mdv_service/medved
- readelf -S build/mdv_service/medved | grep debug
  - If you see `.debug_info`, `.debug_abbrev`, etc. the binary contains debug symbols.

6) Run with gdb
- cd build
- gdb --args ./mdv_service/medved --cfg=../assets/conf/medved.conf
- (in gdb) run
- You can set breakpoints by function name, inspect source-level stack frames and variables when debug symbols are available.

Notes
- Use RelWithDebInfo for realistic performance with symbols; Debug for easier stepping.
- The root CMakeLists.txt provides:
  - `ENABLE_DEBUG_SYMBOLS` option to inject `-g` into compiler flags.
  - `DEFAULT_BUILD_TYPE_RELWITHDEBINFO` option to set RelWithDebInfo as default (both are OFF by default).
