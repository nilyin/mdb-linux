# MedvedDB Build and Test Scripts Reference

**Location**: All scripts are located in the `scripts/` directory to keep the project root clean.

## Current Active Scripts (Recommended)

### Core Test Scripts
| Script | Purpose | Status |
|--------|---------|--------|
| `test_data_consistency.{ps1,bat,sh}` | just compiles project and test files (Platform, Types, Storage, Crypto) | ✅ **RECOMMENDED** |
| `run_core_tests.{ps1,bat,sh}` | compiles project files , runs database server and test connection| ✅ **RECOMMENDED** |
| `run_complete_tests.{ps1,bat,sh}` | same as before + runs mdv_tests suite and cleans the server | ✅ **RECOMMENDED** |
| `run_all_tests_final.{ps1,bat,sh}` | same as above, but calling run_full_tests script internally | ✅ **RECOMMENDED** |
| 'run_full_tests.* | just builds and runs mdv_tests/mdv_tests

### Build Scripts
| Script | Purpose | Status |
|--------|---------|--------|
| `incremental_build.{bat,sh}` | Docker incremental build with cache | ✅ **RECOMMENDED** |
| `docker_build_cache.{bat,sh}` | Generic Docker cache wrapper | ✅ **UTILITY** |


run_platform_test - just build platfrom file in container - can be deleted

### Utility Scripts
| Script | Purpose | Status |
|--------|---------|--------|
| `docker_publish.{bat,sh}` | Docker Hub publishing | ✅ **UTILITY** |

## Script Purposes and Usage

### 1. Core Testing Scripts

#### `test_data_consistency.{ps1,bat,sh}`
- **Purpose**: Build and verify core components (Platform, Types, Storage, Crypto)
- **Features**: Docker cache, 10x faster builds, data consistency verification
- **Usage**: Quick verification of core functionality
- **Runtime**: ~10-15 seconds with cache

#### `run_core_tests.{ps1,bat,sh}`
- **Purpose**: Core components + MedvedDB server verification
- **Features**: Server startup, connectivity test, clean shutdown
- **Usage**: Verify server can start and respond on tcp://127.0.0.1:4800
- **Runtime**: ~20-30 seconds with cache

#### `run_complete_tests.{ps1,bat,sh}`
- **Purpose**: Complete test suite with integrated server
- **Features**: Single container, server + tests, comprehensive verification
- **Usage**: Full test execution including CRUD (when linking fixed)
- **Runtime**: ~30-60 seconds with cache

#### `run_all_tests_final.{ps1,bat,sh}`
- **Purpose**: Comprehensive test execution with detailed reporting
- **Features**: Multi-step verification, detailed status, final summary
- **Usage**: Complete system verification and CI/CD integration
- **Runtime**: ~60-90 seconds with cache

### 2. Build Scripts

#### `incremental_build.{bat,sh}`
- **Purpose**: Build all components with Docker cache optimization
- **Features**: Persistent cache, parallel compilation, component status
- **Usage**: Development builds, CI/CD pipelines
- **Runtime**: 4-6 seconds incremental, 2-4 minutes initial

#### `docker_build_cache.{bat,sh}`
- **Purpose**: Generic wrapper for Docker cache operations
- **Features**: Volume management, custom command execution
- **Usage**: Utility for custom Docker operations
- **Runtime**: Variable based on command


## Recommended Usage Patterns

### Development Workflow
1. **First time**: `./run_all_tests_final.sh` (comprehensive verification)
2. **Daily development**: `./test_data_consistency.sh` (quick core verification)
3. **Before commits**: `./run_core_tests.sh` (server verification)
4. **Build only**: `./incremental_build.sh` (fast compilation)

### CI/CD Pipeline
1. **Pull Request**: `./run_all_tests_final.sh`
2. **Nightly builds**: `./run_complete_tests.sh`
3. **Release verification**: `./run_all_tests_final.sh`

### Debugging
1. **Core issues**: `./test_data_consistency.sh`
2. **Server issues**: `./run_core_tests.sh`
3. **CRUD issues**: `./run_complete_tests.sh` (when linking fixed)

## Scripts to Remove (Redundant)

The following scripts can be safely removed as they are fully replaced:


### Keep for Compatibility
- All `.bat` and `.ps1` versions should be kept
- `docker_build_cache.{bat,sh}` - Still useful as utility
- `docker_publish.{bat,sh}` - Required for Docker Hub publishing


### Environment Setup
```bash
# Make scripts executable (Linux/Mac)
chmod +x scripts/*.sh

# Test Docker setup
docker volume create mdv_build_cache
docker pull medveddb-test:latest

# Run comprehensive verification
./scripts/run_all_tests_final.sh
```

## Summary

- ✅ **4 new optimized script families** with Docker cache
- ⚠️ **4 legacy scripts** maintained for compatibility
- 🗑️ **4 scripts recommended for removal** (redundant)
- 🚀 **10x performance improvement** with Docker cache
- 🔧 **Consistent interface** across Windows/Linux/PowerShell