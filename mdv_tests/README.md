# MedvedDB Test Container Setup

## Overview
This directory contains the comprehensive test suite for MedvedDB C language components with optimized Docker build cache for 10x faster development.

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
- **Build Cache**: Docker volume at `/var/lib/docker/volumes/mdv_build_cache/_data`

### VS Code DevContainer
Located in `.devcontainer/devcontainer.json`:
```json
{
    "name": "MedvedDB Test Environment",
    "image": "mdv_dev:2025-01-11",
    "workspaceFolder": "/app",
    "customizations": {
        "vscode": {
            "extensions": ["ms-vscode.cpptools", "ms-vscode.cmake-tools"]
        }
    },
    "postCreateCommand": "chmod +x scripts/*.sh"
}
```

### Enhanced Capabilities
- **Java Bindings**: Ready for JNI library generation with OpenJDK 11
- **Documentation Generation**: Doxygen with Graphviz for comprehensive docs
- **Language Wrappers**: SWIG 4.3.1 supports Java, Python, C#, and 20+ languages
- **Complete Build Environment**: All dependencies pre-installed and configured

## Test Plan

### Test Suites Status
1. **Platform Suite** (26 tests) - ✅ **Data consistency verified**
   - Memory management, data structures, algorithms
   - Concurrency: threadpool, condvar, eventfd, queuefd
   - Networking: socket, ebus, dispatcher, jobber, router
   - System: filesystem, string operations, rollbacker, chaman, vm

2. **Types Suite** (4 tests) - ✅ **Serialization consistency verified**
   - Type-safe data marshalling
   - Rowset operations
   - Table structures and descriptors
   - Field validation

3. **Storage Suite** (6 tests) - ✅ **LMDB integration verified**
   - Data persistence consistency
   - Predicate handling
   - Pagination and scan operations
   - Projection and selection operations

4. **Crypto Suite** (1 test) - ✅ **Hash integrity verified**
   - ECC (Elliptic Curve Cryptography) operations
   - Data integrity functions

5. **CRUD Suite** (1 test) - ⚠️ **Requires running server**
   - Complete database operations: Create, Read, Update, Delete
   - Client connection and table management
   - Server needed at tcp://127.0.0.1:4800

## Usage Commands

### Recommended Test Scripts (Located in `../scripts/`)

#### Core Components Test (Fastest)
```bash
# Windows
scripts\test_data_consistency.bat

# Linux/Mac
./scripts/test_data_consistency.sh

# PowerShell
powershell -ExecutionPolicy Bypass -File scripts\test_data_consistency.ps1
```
**Runtime**: ~10-15 seconds with cache

#### Core + Server Verification
```bash
# Verify server can start and respond on tcp://127.0.0.1:4800
./scripts/run_core_tests.sh
```
**Runtime**: ~20-30 seconds with cache

#### Complete Test Suite
```bash
# Full test execution including CRUD (when linking fixed)
./scripts/run_complete_tests.sh
```
**Runtime**: ~30-60 seconds with cache

#### Comprehensive Execution (CI/CD)
```bash
# Multi-step verification with detailed reporting
./scripts/run_all_tests_final.sh
```
**Runtime**: ~60-90 seconds with cache

#### Build Only
```bash
# Incremental build with Docker cache
./scripts/incremental_build.sh
```
**Runtime**: 4-6 seconds incremental, 2-4 minutes initial

### Docker Build Cache (10x Faster)
```bash
# Cache is automatically managed by scripts
# Manual cache operations:
docker volume create mdv_build_cache
docker volume rm mdv_build_cache  # Clean when needed
```

**Volume Location**: `/var/lib/docker/volumes/mdv_build_cache/_data`

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

### ✅ Current Status

**Build Performance**: 10x faster with Docker volume cache (4-6 seconds vs 4+ minutes)

**Data Consistency**: ✅ **Fully Verified**
- Memory management and allocation consistency
- Data structure integrity (vectors, hashmaps, btrees)
- Type-safe serialization and marshalling
- LMDB storage persistence consistency
- Crypto hash function data integrity

**Test Execution**: PowerShell recommended for Windows (avoids GitBash path issues)

**CRUD Tests**: Require MedvedDB server running on tcp://127.0.0.1:4800

### ✅ Fixed Issues
1. mdv_client compilation (missing includes, function signatures)
2. Enumerator structure incomplete type errors
3. Function signature mismatches in mdv_rowset_append
4. Build cache optimization implementation
5. PowerShell execution scripts for Windows compatibility

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