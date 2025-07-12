# MedvedDB Quick Start Guide

## Prerequisites
- Docker installed
- Git repository cloned

## 1. Fast Build & Test (Recommended)
```bash
# Windows
cmd /c test_data_consistency.bat

# Linux/Mac  
./test_data_consistency.sh
```
**Result**: 4-second build with data consistency verification

## 2. Development Workflow
```bash
# Create build cache (one time)
docker volume create mdv_build_cache

# Incremental development builds
docker run --rm -v "%cd%":/app -v mdv_build_cache:/app/build -w /app medveddb-test:latest bash -c "cd build && make mdv_client -j4"

# Run client
docker run --rm -v "%cd%":/app -v mdv_build_cache:/app/build -w /app medveddb-test:latest bash -c "cd build && ./mdv_client/mdv tcp://127.0.0.1:4800"
```

## 3. Clean Cache (When Needed)
```bash
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
✅ Client compilation (fixed signatures)