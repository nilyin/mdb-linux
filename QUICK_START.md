# MedvedDB Quick Start Guide

## Prerequisites
- Docker installed
- Git repository cloned

## 1. Fast Build & Test (Recommended)
```bash
# Windows
scripts\test_data_consistency.bat

# Linux/Mac  
./scripts/test_data_consistency.sh

# PowerShell
powershell -ExecutionPolicy Bypass -File scripts\test_data_consistency.ps1
```
**Result**: 4-second build with data consistency verification

## 2. Complete Test Suite
```bash
# Core components + server verification
./scripts/run_core_tests.sh

# Complete test suite with CRUD
./scripts/run_complete_tests.sh

# Comprehensive execution (CI/CD)
./scripts/run_all_tests_final.sh
```

## 3. Development Workflow
```bash
# Incremental build only
./scripts/incremental_build.sh

# Quick core verification
./scripts/test_data_consistency.sh

# Clean cache when needed
docker volume rm mdv_build_cache
```

## Performance Comparison
- **Without cache**: 4+ minutes full rebuild
- **With cache**: 4-10 seconds incremental build
- **Speedup**: 10x faster development cycle

## Verified Components
✅ Platform libraries (memory, data structures)  
✅ Type system (serialization, validation)  
✅ Storage layer (LMDB integration)  
✅ Crypto layer (hash integrity)  
✅ MedvedDB server (tcp://127.0.0.1:4800)  
✅ Client compilation (fixed signatures)

## Script Reference
See `scripts/SCRIPT_REFERENCE.md` for complete documentation of all available scripts.