# Developer Documentation: Row ID and Data Manipulation

This document provides a summary of the project's structure, recent changes to introduce row IDs, and examples of how to use the new functionality in Java.

## Project Structure Summary

The project is a peer-to-peer distributed database system designed for IoT and edge computing environments. It is built with a modular C architecture, and uses SWIG to generate Java bindings.

The main modules are:
-   **`mdv_core`**: The central module that orchestrates the other components.
-   **`mdv_storage`**: The storage engine layer, currently using LMDB.
-   **`mdv_net`**: Manages the peer-to-peer networking.
-   **`mdv_api`**: Defines the public API for interacting with the database.
-   **`mdv_bindings`**: Generates language bindings, specifically for Java (JNI) using SWIG.

For more details, please refer to the `architecture.md` file in the memory bank.
## Design Rationale: Why `objid`?

The decision to expose the row identifier as a specific `objid` class in Java, rather than a generic `String` or `java.util.UUID`, is based on performance, type safety, and API consistency.

- **Performance:** `mdv_objid` is the native, internal identifier used by the storage engine. Exposing it directly avoids the significant overhead of converting the binary ID to a string (and back again) for every database operation.
- **Type Safety:** `mdv_objid` is a 12-byte structure, while a standard Java `UUID` is 16 bytes. Using a specific class ensures that only validly structured identifiers are used, preventing errors.
- **API Consistency:** The Java API is designed to be a thin wrapper around the C API. Mirroring the native `mdv_objid` type provides a clear and consistent development experience across both layers.

## Row ID Implementation

The `mdv_objid` struct represents a unique identifier for a row in the database. This ID is now exposed through the Java API via the `row_id()` method on the `RowSetEnumerator` class.

The following changes were made to implement this functionality:
-   The `mdv_enumerator` interface was extended to include a `row_id()` function.
-   The `mdv_rowset` implementation was updated to store and retrieve the `mdv_objid` for each row.
-   The `mdv_select` operator was modified to extract the `mdv_objid` from the storage layer.
-   The SWIG interface was updated to expose the `row_id()` method to Java.

## Java Usage Examples

The following examples demonstrate how to use the new `row_id()` method and the new `delete()` and `update()` methods in Java.

### Retrieving Row IDs

This example shows how to iterate over a `RowSet` and retrieve the unique ID for each row.

```java
import mdv.RowSet;
import mdv.RowSetEnumerator;
import mdv.objid;

// Assuming 'rowset' is a valid RowSet object
try (RowSetEnumerator enumerator = rowset.get_enumerator()) {
    while (enumerator.hasNext()) {
        objid rowId = enumerator.row_id();
        System.out.println("Row ID: " + rowId.to_str());
    }
}
```

### Deleting a Row by ID

This example shows how to delete a row from a table using its unique ID.

```java
import mdv.Client;
import mdv.Table;
import mdv.objid;
import mdv.err;

// Assuming 'client' is a valid Client object,
// 'table' is a valid Table object,
// and 'rowId' is the objid of the row to delete.
if (table.delete(client, rowId) == err.MDV_OK) {
    System.out.println("Row deleted successfully.");
} else {
    System.err.println("Error deleting row.");
}
```

### Updating a Row by ID

This example shows how to update a row in a table using its unique ID.

```java
import mdv.Client;
import mdv.Table;
import mdv.RowSet;
import mdv.objid;
import mdv.err;

// Assuming 'client' is a valid Client object,
// 'table' is a valid Table object,
// 'rowId' is the objid of the row to update,
// and 'newRowSet' is a RowSet containing the new data.
if (table.update(client, rowId, newRowSet) == err.MDV_OK) {
    System.out.println("Row updated successfully.");
} else {
    System.err.println("Error updating row.");
}
## Design Rationale Summary: Why `objid`?

The decision to expose the row identifier as a specific `objid` class in the Java API, rather than a more generic `String`, was based on three key factors:

1.  **Performance:** Using the native `mdv_objid` struct avoids the costly overhead of converting the ID to and from a string format during database operations, which is critical for performance when handling a large number of rows.
2.  **Type Safety:** `mdv_objid` has a specific 12-byte structure. Using a dedicated class instead of a generic type like `String` or a 16-byte Java `UUID` ensures that only correctly-structured identifiers are used, preventing bugs and ensuring data integrity.
3.  **API Consistency:** The Java API is designed as a thin wrapper around the C core. By mirroring the native `mdv_objid` type, the API remains consistent across layers, making it easier for developers to understand and debug.