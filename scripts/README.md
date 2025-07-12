# MedvedDB Scripts Directory

## Quick Reference

### Recommended Scripts
- `test_data_consistency.{ps1,bat,sh}` - Core components test (~10-15 sec)
- `run_core_tests.{ps1,bat,sh}` - Core + server verification (~20-30 sec)  
- `run_complete_tests.{ps1,bat,sh}` - Complete test suite (~30-60 sec)
- `run_all_tests_final.{ps1,bat,sh}` - Comprehensive execution (~60-90 sec)
- `incremental_build.{bat,sh}` - Build only (~4-6 sec)

### Usage Examples
```bash
# Quick core verification
./scripts/test_data_consistency.sh

# Complete testing
./scripts/run_complete_tests.sh

# Windows
scripts\test_data_consistency.bat

# PowerShell
powershell -ExecutionPolicy Bypass -File scripts\test_data_consistency.ps1
```

See `SCRIPT_REFERENCE.md` for complete documentation.