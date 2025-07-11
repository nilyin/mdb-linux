# SWIG Java Iterator Memory Management Fix

## Problem Description

The original SWIG-generated `RowSetEnumerator` wrapper for the C language iterator function `mdv.RowSetEnumerator` was causing native crashes without Java exceptions. This issue was similar to problems described in:
- https://github.com/swig/swig/issues/1685 (Java std_map templates iterator typemap issues)
- https://github.com/swig/swig/issues/2049 (Segfault due to missing null pointer checks)

### Root Cause

The crash occurred because:
1. **Premature Garbage Collection**: The Java garbage collector could reclaim the `RowSetEnumerator` object before all native C resources were properly released
2. **Double-Free Issues**: When the Java-side `RowSetEnumerator` was garbage collected, its finalizer would call the native destructor, which could attempt to free already-freed C memory
3. **Lack of Explicit Resource Management**: The original implementation relied on finalizers and automatic cleanup, which is unreliable for native resources

### Symptoms
- Native crashes without Java stack traces
- Application termination at `mdv.RowSetEnumerator` calls
- No Java exceptions thrown before crash

## Solution

Modified the SWIG interface (`assets/swig/mdv/mdv_rowset.i`) to:

1. **Implement Java Standard Interfaces**: `RowSetEnumerator` now implements `java.util.Iterator<Row>` and `java.lang.AutoCloseable`
2. **Explicit Resource Management**: Added `close()` method for deterministic native resource cleanup
3. **Prevent Default Destructors**: Used `%nodefaultctor` and `%nodefaultdtor` to control object lifecycle
4. **Iterator Pattern Compliance**: Proper `hasNext()` and `next()` methods following Java Iterator contract

## Usage Changes

### Old Pattern (Problematic)
```java
RowSetEnumerator it = result.enumerator();

while (it.next()) {
    Row row = it.current();
    // Process row
    row.delete();
}

it.delete();  // Manual cleanup required
```

### New Pattern 1: Try-with-resources (Recommended)
```java
try (RowSetEnumerator it = result.enumerator()) {
    while (it.hasNext()) {
        Row row = it.next();
        // Process row
        row.delete();
    }
} // Automatic cleanup via AutoCloseable
```

### New Pattern 2: Enhanced for-each loop
```java
try (RowSetEnumerator it = result.enumerator()) {
    for (Row row : it) {
        // Process row
        row.delete();
    }
} // Automatic cleanup
```

### New Pattern 3: Manual cleanup (if try-with-resources not available)
```java
RowSetEnumerator it = result.enumerator();
try {
    while (it.hasNext()) {
        Row row = it.next();
        // Process row
        row.delete();
    }
} finally {
    it.close();  // Explicit cleanup
}
```

## Benefits

1. **Crash Prevention**: Eliminates native crashes due to improper resource management
2. **Memory Safety**: Deterministic cleanup of native resources
3. **Java Idioms**: Follows standard Java patterns for resource management
4. **Backward Compatibility**: Old `moveNext()` and `current()` methods still available
5. **Exception Safety**: Proper cleanup even when exceptions occur

## Migration Notes

- Replace `it.next()` calls with `it.hasNext()` in while loops
- Replace `it.current()` calls with `it.next()` 
- Replace `it.delete()` with `it.close()` or use try-with-resources
- The `moveNext()` and `current()` methods are still available for backward compatibility