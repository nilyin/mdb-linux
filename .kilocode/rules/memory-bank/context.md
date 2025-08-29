# Current Context

**Recent Major Changes:**

## 1. ✅ RESOLVED: "Table not found" Critical Bug
**Problem Resolved:**
- Server reported "Table not found" errors despite valid SELECT requests
- Issue occurred on 2nd+ test runs without LMDB database cleaning
- Root cause: Timing window between TR log append and table registration

**Changes Made:**
- **Direct Table Registration**: Modified `mdv_tablespace_trlog_apply` to immediately register tables in `mdv_tables` after TR log append
- **Robust Cursor Handling**: Fixed `mdv_cursor_open_explicit` to handle empty databases gracefully
- **LMDB Map Creation**: Ensured OBJECTS subdatabase creation with proper flags
- **Performance Test Fix**: Fixed `mdv_perf_test_single_updates` early return bug

**Files Modified:**
- `mdv_core/storage/mdv_tablespace.c` - Direct table registration
- `mdv_storage/mdv_lmdb.c` - Empty database cursor handling
- `mdv_storage/mdv_2pset.c` - LMDB map creation robustness
- `mdv_tests/mdv_perf.c` - Performance test bug fix

**Impact:**
- ✅ Tables visible immediately after creation
- ✅ SELECT operations work on all test runs
- ✅ Rowdata operations handle empty databases
- ✅ Performance tests show accurate metrics

## 2. 🔄 ONGOING: SWIG Java Iterator Fix
**Status:** Documentation created, pending Java bindings rebuild

**Problem Context:**
- Native crashes without Java exceptions when using `RowSetEnumerator`
- Issue was caused by premature garbage collection and improper native resource management

**Changes Made:**
- Modified `assets/swig/mdv/mdv_rowset.i` to implement Java `Iterator<Row>` and `AutoCloseable` interfaces
- Added explicit `close()` method for deterministic native resource cleanup
- Implemented proper `hasNext()` and `next()` methods following Java Iterator contract
- Added `%nodefaultctor` and `%nodefaultdtor` directives for controlled object lifecycle
- Maintained backward compatibility with `moveNext()` and `current()` methods

**Documentation Created:**
- `docs/SWIG_Iterator_Fix.md` - Technical problem description and solution overview
- `docs/Iterator_Migration_Example.md` - Detailed code migration examples showing old vs new patterns

**Next Steps:**
- Rebuild Java bindings to test the fix
- Update any existing Java code to use new iterator patterns (try-with-resources recommended)
- Monitor for any remaining iterator-related issues

## 3. 📚 Documentation Updates
**Completed:**
- Updated `docs/table_notfound_issue.md` with complete solution summary
- Documented all fixes, root causes, and verification results
- Added database operations flow diagrams
- Included performance test fixes and explanations

**Current Status:**
- All critical database operation issues resolved
- System stable for multiple test runs
- Performance testing fully functional
- Java bindings fix pending implementation