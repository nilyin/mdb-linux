# Current Context

**Recent Major Change:** Fixed critical SWIG Java Iterator crash issue with `mdv.RowSetEnumerator`.

**Problem Resolved:**
- Native crashes without Java exceptions when using `RowSetEnumerator`
- Issue was caused by premature garbage collection and improper native resource management
- Similar to known SWIG issues with iterator wrapping and null pointer handling

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