# MedvedDB Symbol Conflict Analysis

## Problem: Multiple Definition of `mdv_select`

### **Root Cause**
Two completely different functions with the same name exist in different libraries:

1. **Server-side** (`mdv_storage/ops/mdv_select.c`):
   ```c
   mdv_op * mdv_select(mdv_op *src, mdv_predicate *predicate);
   ```
   - **Purpose**: Internal storage operation for filtering DB entries
   - **Returns**: Storage operation object
   - **Used by**: Storage layer internally

2. **Client-side** (`mdv_api/mdv_client.c`):
   ```c
   mdv_rowset * mdv_select(mdv_client *client, mdv_table *table, 
                          mdv_bitset *fields, char const *filter);
   ```
   - **Purpose**: Public API for client SELECT queries
   - **Returns**: Result set for client consumption
   - **Used by**: Client applications, SWIG bindings

### **SWIG Impact**
The SWIG wrapper (`assets/swig/mdv/mdv_client.i`) exposes the **client-side** function:
```c
mdv_rowset * select(mdv_table *table, mdv_bitset *fields, char const *filter)
{
    return mdv_select($self, table, fields, filter);  // Calls client mdv_select
}
```

## Recommended Solution

### **Option 1: Rename Client Function (RECOMMENDED)**
- **Change**: `mdv_select` → `mdv_client_select` in client API
- **Files to update**:
  1. `mdv_api/mdv_client.h` - Function declaration
  2. `mdv_api/mdv_client.c` - Function definition  
  3. `mdv_tests/mdv_crud.c` - Test usage
  4. `assets/swig/mdv/mdv_client.i` - SWIG wrapper call
- **Impact**: SWIG bindings continue to expose `select()` method, internal call changes
- **Effort**: ~30 minutes
- **Risk**: Low (only internal API change)

### **Option 2: Rename Storage Function**
- **Change**: `mdv_select` → `mdv_storage_select` in storage layer
- **Files to update**: Storage layer files only
- **Impact**: No SWIG changes needed
- **Effort**: ~20 minutes  
- **Risk**: Medium (internal storage API change)

### **Option 3: Use Static Linkage**
- **Change**: Make storage `mdv_select` static
- **Impact**: May break storage layer architecture
- **Risk**: High

## Implementation Plan (Option 1)

### Step 1: Update Client API (5 minutes)
```c
// In mdv_api/mdv_client.h
mdv_rowset * mdv_client_select(mdv_client *client, mdv_table *table, 
                              mdv_bitset *fields, char const *filter);

// In mdv_api/mdv_client.c  
mdv_rowset * mdv_client_select(mdv_client *client, mdv_table *table,
                              mdv_bitset *fields, char const *filter)
```

### Step 2: Update SWIG Wrapper (5 minutes)
```c
// In assets/swig/mdv/mdv_client.i
mdv_rowset * select(mdv_table *table, mdv_bitset *fields, char const *filter)
{
    return mdv_client_select($self, table, fields, filter);
}
```

### Step 3: Update Test (5 minutes)
```c
// In mdv_tests/mdv_crud.c
mdv_rowset *select_rowset = mdv_client_select(client, table, 0, "");
```

### Step 4: Verify Build (15 minutes)
- Test compilation of all components
- Verify CRUD tests link and execute
- Test SWIG bindings still work

## Benefits of Option 1
- ✅ **SWIG Compatibility**: Java/Python/C# APIs unchanged (`client.select()`)
- ✅ **Clear Naming**: `mdv_client_select` vs `mdv_select` (storage)
- ✅ **Minimal Impact**: Only 4 files to change
- ✅ **Backward Compatible**: SWIG bindings maintain same interface

## Estimated Time: 30 minutes total