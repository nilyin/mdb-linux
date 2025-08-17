Row ID Refactoring Plan
Problem Statement
Current mdv_rowset_append expects client-provided row IDs, causing segmentation faults when NULL is passed. Row IDs should be generated server-side during transaction log processing.

Architecture Analysis
Row IDs: Generated in mdv_tablespace.c via mdv_rowdata_reserve()

LMDB Keys: Complete mdv_objid structure (node + id)

Client rowsets: In-memory only, don't need persistent IDs

Server persistence: Row IDs created during WAL processing

Refactoring Plan


1. Phase 1: Remove row_ids Parameter
Files affected:

mdv_types/mdv_rowset.h

mdv_types/mdv_rowset.c

mdv_api/mdv_client.c

Changes:

// OLD signature
size_t mdv_rowset_append(mdv_rowset *rowset, mdv_objid const *row_ids, mdv_data const **rows, size_t count);

// NEW signature  
size_t mdv_rowset_append(mdv_rowset *rowset, mdv_data const **rows, size_t count);


2. Phase 2: Fix mdv_rowset_impl_append (REVISED)
File: mdv_types/mdv_rowset.c

Changes:

Remove row_ids parameter from function signature

Remove line: entry->row_id = row_ids[i]; (causes segfault)

Initialize entry->row_id to empty: entry->row_id = (mdv_objid){0};

Keep all other logic unchanged

Rationale: In-memory rowsets don't need persistent row IDs. IDs are generated later during transaction log processing in mdv_tablespace.c.


3. Phase 3: Update Client API Calls
File: mdv_api/mdv_client.c

Changes:

// OLD call in mdv_insert()
mdv_rowset_append(rowset, NULL, rows, count)

// NEW call
mdv_rowset_append(rowset, rows, count)



4. Phase 4: Fix Test Cases
Files affected:

mdv_tests/mdv_types/mdv_rowset.h

All test files using mdv_rowset_append

Changes:

// OLD test call
mu_check(mdv_rowset_append(rowset, NULL, rows, sizeof rows / sizeof *rows) == sizeof rows / sizeof *rows);

// NEW test call
mu_check(mdv_rowset_append(rowset, rows, sizeof rows / sizeof *rows) == sizeof rows / sizeof *rows);


5. Phase 5: Verify Row ID Access (No Changes)
Preserve existing functionality:

mdv_enumerator_row_id() - Returns mdv_objid const *

mdv_delete(mdv_client *client, mdv_table *table, mdv_objid const *row_id) - Unchanged

mdv_update(mdv_client *client, mdv_table *table, mdv_objid const *row_id, mdv_rowset *rowset) - Unchanged


Implementation Order
Update function signatures in header files

Fix mdv_rowset_impl_append implementation

Update client API calls to remove NULL parameter

Fix all test cases

Build and test to verify segfault is resolved

Benefits
Eliminates segfaults - No more NULL pointer dereference

Maintains architecture - Row IDs still generated in proper location

Preserves API compatibility - Delete/update operations unchanged

Keeps row ID access - Enumerators provide IDs for operations

Minimal changes - Only removes problematic parameter

Files Not Changed
WAL transaction log processing (mdv_tablespace.c)

Row ID generation logic (mdv_rowdata_reserve)

LMDB key structure (mdv_objid)

Enumerator row ID access methods

Delete/update operation signatures

