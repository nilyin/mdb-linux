# MedvedDB Test Container Setup

## Overview
This directory contains the comprehensive test suite for MedvedDB C language components, configured to run in a Docker container environment with VS Code Remote Container support.

## Container Configuration

### Docker Image
- **Image**: `medveddb-test:latest`
- **Base OS**: Debian Bullseye Slim
- **Build Tools**: GCC 10.2.1, CMake 3.18.4, Git
- **Java**: OpenJDK 11.0.27
- **Documentation**: Doxygen 1.9.1, Graphviz
- **Language Bindings**: SWIG 4.3.1 (with PCRE2 support)
- **Build Dependencies**: Flex, Bison, PCRE development libraries
- **Network Tools**: Netcat for server connectivity testing

### VS Code DevContainer
Located in `.devcontainer/devcontainer.json`:
```json
{
    "name": "MedvedDB Test Environment",
    "image": "medveddb-test:latest",
    "workspaceFolder": "/app",
    "customizations": {
        "vscode": {
            "extensions": ["ms-vscode.cpptools", "ms-vscode.cmake-tools"]
        }
    },
    "postCreateCommand": "chmod +x run_tests.sh && chmod +x run_platform_tests.sh"
}
```

### Enhanced Capabilities
- **Java Bindings**: Ready for JNI library generation with OpenJDK 11
- **Documentation Generation**: Doxygen with Graphviz for comprehensive docs
- **Language Wrappers**: SWIG 4.3.1 supports Java, Python, C#, and 20+ languages
- **Complete Build Environment**: All dependencies pre-installed and configured

## Test Plan

### Test Suites (38 total tests)
1. **Platform Suite** (26 tests) - ✅ Build successful
   - Data structures: stack, vector, queue, hashmap, list, btree
   - Concurrency: threadpool, condvar, eventfd, queuefd
   - Networking: socket, ebus, dispatcher, jobber, router
   - Algorithms: bloom filter, bitset, lru cache, topology, mst
   - System: filesystem, string operations, rollbacker, chaman, vm

2. **Types Suite** (4 tests) - ❌ Compilation issues
   - Serialization mechanisms
   - Rowset operations
   - Table structures
   - Table descriptors

3. **Crypto Suite** (1 test) - ❌ Dependency issues
   - ECC (Elliptic Curve Cryptography) operations

4. **Storage Suite** (6 tests) - ❌ Dependency issues
   - Predicate handling
   - Pagination
   - Scan operations (sequential)
   - Projection operations (range and indices)
   - Selection operations

5. **CRUD Suite** (1 test) - ❌ Requires server + compilation fixes
   - Complete database operations: Create, Read, Update, Delete
   - Client connection and table management

## Usage Commands

### Build and Run All Tests
```bash
./run_tests.sh
```

### Build Platform Tests Only
```bash
./run_platform_tests.sh
```

### Manual Docker Execution
```bash
# Clean build and run tests
docker run --rm -v "$(pwd)":/app -w /app sha256:716a32fd6c93631972b5674ef88a1c9b9c498ded23929fec4102781d2be39bda bash -c "apt-get update -qq && apt-get install -y -qq netcat-openbsd && rm -rf build && ./run_tests.sh"
```

## Test Architecture

### Network vs Direct Access
- **CRUD Tests**: Use network client connection via `mdv_client_connect()`
- **Platform/Storage/Types/Crypto Tests**: Direct library function calls
- **Server Requirement**: Only CRUD tests need running MedvedDB server

### Server Configuration
- **Config File**: `assets/conf/medved.conf`
- **Default Port**: 4800 (TCP)
- **Data Directory**: `./data`
- **Auto-startup**: Handled by test scripts

## Current Status

### ✅ Working Components
- Docker container setup
- CMake build system
- Platform library (26 components)
- Third-party dependencies
- Server configuration

### ❌ Known Issues
1. Missing include dependencies (`mdv_objid.h`)
2. Struct definition mismatches in `mdv_rowlist_entry`
3. Circular dependencies between platform and types layers

### 🔧 Required Fixes
1. Fix struct definitions in `mdv_types/mdv_rowset.c`
2. Resolve circular dependencies
3. Complete include fixes for missing headers

## Test Framework
- **Framework**: MinUnit (lightweight C testing)
- **Execution**: Sequential with timing and reporting
- **Output**: Pass/fail status with detailed error messages

## Files Structure
```
mdv_tests/
├── README.md                 # This file
├── main.c                   # Test runner entry point
├── minunit.h               # Test framework
├── mdv_*.h                 # Test suite headers
├── mdv_*.c                 # Test implementations
├── mdv_platform/           # Platform component tests
├── mdv_types/              # Type system tests
├── mdv_storage/            # Storage layer tests
└── mdv_crypto/             # Cryptography tests
```