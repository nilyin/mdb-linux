# Iterator Migration Example

## Your Original Code (Problematic)

```java
RowSetEnumerator it = result.enumerator();                      // Get result set iterator

// check if all parameters not null or empty
if (dir != null && !dir.isEmpty() && did != null && !did.isEmpty()) {

    while (it.next()) {
        Row row = it.current();

        FileMdbDao fileDAO = new FileMdbDao();
        
        Log.d(TAG, FTAG + ": Row read: " + row.toString());

        int k = 0; // - индекс столбца c нужными нам данными в полученном raw
        if (row.getString(dir_index).contains(dir) && row.getString(did_index).equals(did)) {

            if (used_indexes.contains(0)) {fileDAO.setFileId(row.getString(0)); k++;}
            if (used_indexes.contains(1)) {fileDAO.setDeviceId(row.getString(k)); k++;}
            if (used_indexes.contains(2)) {fileDAO.setDiskName(row.getString(k)); k++;}
            if (used_indexes.contains(3)) {fileDAO.setNameShort(row.getString(k)); k++;}
            if (used_indexes.contains(4)) {fileDAO.setDirFullPath(row.getString(k)); k++;}
            if (used_indexes.contains(5)) {fileDAO.setModifiedTime(row.getInt64(k)); k++;}
            if (used_indexes.contains(6)) {fileDAO.setExtension(row.getString(k)); k++;}
            if (used_indexes.contains(7)) {fileDAO.setFileSize(row.getInt64(k)); k++;}

            file_list.add(fileDAO);
            fileDAO = null;
        }

        row.delete();
    }
} else if (dir != null && !dir.isEmpty()) {
    // Similar pattern repeated...
}

it.delete();                                                    // Delete result set iterator
result.delete();                                                // Delete result set
bitset.delete();                                                // Delete bit set
```

## Migrated Code (Safe)

### Option 1: Try-with-resources (Recommended)

```java
// Use try-with-resources for automatic cleanup
try (RowSetEnumerator it = result.enumerator()) {
    
    // check if all parameters not null or empty
    if (dir != null && !dir.isEmpty() && did != null && !did.isEmpty()) {

        while (it.hasNext()) {
            Row row = it.next();  // Changed: it.next() now returns the Row directly

            FileMdbDao fileDAO = new FileMdbDao();
            
            Log.d(TAG, FTAG + ": Row read: " + row.toString());

            int k = 0;
            if (row.getString(dir_index).contains(dir) && row.getString(did_index).equals(did)) {

                if (used_indexes.contains(0)) {fileDAO.setFileId(row.getString(0)); k++;}
                if (used_indexes.contains(1)) {fileDAO.setDeviceId(row.getString(k)); k++;}
                if (used_indexes.contains(2)) {fileDAO.setDiskName(row.getString(k)); k++;}
                if (used_indexes.contains(3)) {fileDAO.setNameShort(row.getString(k)); k++;}
                if (used_indexes.contains(4)) {fileDAO.setDirFullPath(row.getString(k)); k++;}
                if (used_indexes.contains(5)) {fileDAO.setModifiedTime(row.getInt64(k)); k++;}
                if (used_indexes.contains(6)) {fileDAO.setExtension(row.getString(k)); k++;}
                if (used_indexes.contains(7)) {fileDAO.setFileSize(row.getInt64(k)); k++;}

                file_list.add(fileDAO);
                fileDAO = null;
            }

            row.delete();
        }
    } else if (dir != null && !dir.isEmpty()) {

        while (it.hasNext()) {
            Row row = it.next();  // Changed: it.next() now returns the Row directly
            FileMdbDao fileDAO = new FileMdbDao();

            int k = 0;
            if (row.getString(dir_index).startsWith(dir)) {
                if (used_indexes.contains(0)) {fileDAO.setFileId(row.getString(0)); k++;}
                if (used_indexes.contains(1)) {fileDAO.setDeviceId(row.getString(k)); k++;}
                if (used_indexes.contains(2)) {fileDAO.setDiskName(row.getString(k)); k++;}
                if (used_indexes.contains(3)) {fileDAO.setNameShort(row.getString(k)); k++;}
                if (used_indexes.contains(4)) {fileDAO.setDirFullPath(row.getString(k)); k++;}
                if (used_indexes.contains(5)) {fileDAO.setModifiedTime(row.getInt64(k)); k++;}
                if (used_indexes.contains(6)) {fileDAO.setExtension(row.getString(k)); k++;}
                if (used_indexes.contains(7)) {fileDAO.setFileSize(row.getInt64(k)); k++;}
                if (used_indexes.contains(8)) {fileDAO.setFileType(row.getString(k)); k++;}
                if (used_indexes.contains(9)) {fileDAO.setFileCrc(row.getInt64(k));}

                file_list.add(fileDAO);
                fileDAO = null;
            }
            row.delete();
        }

    } else if (did != null && !did.isEmpty()) {

        while (it.hasNext()) {
            Row row = it.next();  // Changed: it.next() now returns the Row directly
            FileMdbDao fileDAO = new FileMdbDao();
            int k = 0;
            if (row.getString(did_index).equals(did)) {
                if (used_indexes.contains(0)) {fileDAO.setFileId(row.getString(0)); k++;}
                if (used_indexes.contains(1)) {fileDAO.setDeviceId(row.getString(k)); k++;}
                if (used_indexes.contains(2)) {fileDAO.setDiskName(row.getString(k)); k++;}
                if (used_indexes.contains(3)) {fileDAO.setNameShort(row.getString(k)); k++;}
                if (used_indexes.contains(4)) {fileDAO.setDirFullPath(row.getString(k)); k++;}
                if (used_indexes.contains(5)) {fileDAO.setModifiedTime(row.getInt64(k)); k++;}
                if (used_indexes.contains(6)) {fileDAO.setExtension(row.getString(k)); k++;}
                if (used_indexes.contains(7)) {fileDAO.setFileSize(row.getInt64(k)); k++;}
                if (used_indexes.contains(8)) {fileDAO.setFileType(row.getString(k)); k++;}
                if (used_indexes.contains(9)) {fileDAO.setFileCrc(row.getInt64(k));}

                file_list.add(fileDAO);
                fileDAO = null;
            }
            row.delete();
        }
    }
} // Iterator automatically closed here via AutoCloseable

// Still need to clean up other resources manually
result.delete();                                                // Delete result set
bitset.delete();                                                // Delete bit set
Log.d(TAG, FTAG + "exited, file list: /n" + file_list.toString());
return file_list;
```

### Option 2: Manual cleanup (if try-with-resources not available)

```java
RowSetEnumerator it = result.enumerator();
try {
    // Same logic as above, but with manual cleanup
    if (dir != null && !dir.isEmpty() && did != null && !did.isEmpty()) {
        while (it.hasNext()) {
            Row row = it.next();
            // ... process row ...
            row.delete();
        }
    }
    // ... other conditions ...
} finally {
    it.close();  // Explicit cleanup instead of it.delete()
}

result.delete();
bitset.delete();
Log.d(TAG, FTAG + "exited, file list: /n" + file_list.toString());
return file_list;
```

### Option 3: Enhanced for-each loop (most concise)

```java
try (RowSetEnumerator it = result.enumerator()) {
    
    if (dir != null && !dir.isEmpty() && did != null && !did.isEmpty()) {
        for (Row row : it) {  // Enhanced for-each loop
            FileMdbDao fileDAO = new FileMdbDao();
            
            Log.d(TAG, FTAG + ": Row read: " + row.toString());

            int k = 0;
            if (row.getString(dir_index).contains(dir) && row.getString(did_index).equals(did)) {
                // ... same processing logic ...
                file_list.add(fileDAO);
            }
            row.delete();
        }
    }
    // ... handle other conditions similarly ...
}

result.delete();
bitset.delete();
Log.d(TAG, FTAG + "exited, file list: /n" + file_list.toString());
return file_list;
```

## Key Changes Summary

1. **`it.next()` → `it.hasNext()`** in while loop condition
2. **`it.current()` → `it.next()`** to get the current row
3. **`it.delete()` → `it.close()`** or use try-with-resources
4. **Wrap in try-with-resources** for automatic cleanup
5. **Keep `row.delete()`** calls - Row objects still need manual cleanup

## Backward Compatibility

If you need to maintain the old pattern temporarily, the old methods are still available:

```java
// Old methods still work (for backward compatibility)
while (it.moveNext()) {  // renamed from next()
    Row row = it.current();
    // ... process row ...
    row.delete();
}
it.close();  // instead of it.delete()