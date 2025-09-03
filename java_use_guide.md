# MedvedDB Java Client Usage Guide

## Overview

This guide provides comprehensive information about using the MedvedDB Java client for database operations. MedvedDB is a peer-to-peer distributed database designed for IoT and edge computing environments.

## Major Findings on Row and Column Manipulation

### Insert Operations

**Row Creation and Field Setting:**
- Rows are created using `new Row(fieldCount)` constructor
- Fields are set using type-specific methods:
  - `row.setString(index, value)` for string fields
  - `row.setInt32(index, value)` for single integer fields
  - `row.setInt32Array(index, array, size)` for integer arrays
  - `row.setBool(index, value)` for boolean fields

**Array Field Handling:**
- For array fields, create `ArrayOfInt32` objects first
- Set individual array elements using `array.set(index, value)`
- Pass the array to the row using `row.setInt32Array(index, array, size)`

**Row Insertion:**
- Create a `RowSet` using `new RowSet(table)`
- Add rows to the set using `rowset.add(row)`
- Insert the entire set using `client.insert(rowset)`

### Select Operations

**Query Execution:**
- Create a `BitSet` to specify which columns to retrieve
- Use `bitset.fill(true)` to select all columns
- Execute query with `client.select(table, bitset, filterString)`

**Result Processing:**
- Results are returned as `RowSet` containing multiple rows
- Iterate through results using `RowSetEnumerator`
- Access individual rows with `enumerator.current()`

**Field Data Retrieval:**
- Get field size using `row.fieldSize(index)`
- Retrieve data using type-specific getters:
  - `row.getString(index)` for strings
  - `row.getInt32Array(index)` for integer arrays
  - `row.getBool(index)` for booleans

**Array Field Processing:**
- Convert `ArrayOfInt32` to regular arrays for comparison
- Iterate through array elements using `array.get(index)`

## Developer Guidelines for Tests

### Available Tests

**JavaSampleClient.java:**
- Basic CRUD operations demonstration
- Table creation with mixed field types
- Data insertion and retrieval
- Data comparison functionality

**CrudTest.java:**
- Additional test cases for CRUD operations
- Various data type combinations

### Compilation and Execution

**Compilation:**
```bash
cd /app/build
javac -cp mdv_bindings/mdv/java/mdv4j.jar ../mdv_bindings/mdv/java/tests/JavaSampleClient.java -d java-tests
```

**Execution:**
```bash
cd /app/build
java -Djava.library.path=mdv_bindings/mdv/java -cp java-tests:mdv_bindings/mdv/java/mdv4j.jar JavaSampleClient
```

**Prerequisites:**
- JNI library must be available (`libmdv4j.so`)
- Java classes must be compiled with correct classpath
- MedvedDB server must be running on `tcp://localhost:4800`

### Test Structure

**Data Comparison:**
- Tests include automatic comparison between inserted and retrieved data
- Detailed logging shows field-by-field comparison results
- Overall success/failure status is reported

**Error Handling:**
- Check return values from database operations
- Handle null results from select operations
- Proper resource cleanup using `delete()` methods

## Row ID Objects Usage

### ObjectId in C/C++ Backend

**Purpose:**
- ObjectId represents unique identifiers for database rows
- Used for targeted operations like delete and update
- Generated automatically by the database engine

**Usage in Backend:**
- Passed to delete and update operations
- Retrieved from query results
- Used for row-level operations

### ObjectId in Java Bindings

**Current Implementation:**
- ObjectId class is available in the Java bindings
- Included in the compiled JAR file (`mdv/ObjectId.class`)
- Methods are exposed: `delete(Table, ObjectId)` and `update(Table, ObjectId, RowSet)`

**Java Usage Pattern:**
```java
// Retrieve ObjectId from row data
Object objId = row.getObjectId(); // Hypothetical method

// Use in operations
boolean deleteResult = client.delete(table, (mdv.ObjectId)objId);
boolean updateResult = client.update(table, (mdv.ObjectId)objId, rowset);
```

**Type Casting:**
- ObjectId requires explicit casting from Object to mdv.ObjectId
- This is necessary due to JNI type mapping limitations

## ToDo: Update and Delete Operations Implementation

### Current Status

**Completed Work:**
- ✅ Added SWIG wrapper declarations for delete and update operations in `mdv_client.i`
- ✅ Rebuilt JNI bindings with new method signatures
- ✅ Verified new methods are available in compiled Client class:
  - `public boolean delete(mdv.Table, mdv.ObjectId)`
  - `public boolean update(mdv.Table, mdv.ObjectId, mdv.RowSet)`
- ✅ Created test code demonstrating delete and update usage
- ✅ Added comprehensive data comparison functionality
- ✅ Enhanced logging for operation results

**Remaining Issues:**

**Compilation Error:**
```
../mdv_bindings/mdv/java/tests/JavaSampleClient.java:276: error: cannot find symbol
                            boolean deleteResult = client.delete(table, (mdv.ObjectId)objId);
                                                                            ^
  symbol:   class ObjectId
  location: class mdv
```

**Root Cause:**
- The `mdv.ObjectId` class is not accessible during compilation
- Despite being present in the JAR file, the class cannot be resolved
- This prevents successful compilation of test applications

**Potential Solutions:**
1. **Classpath Issue:** Verify JAR file contains correct package structure
2. **Import Statement:** Add explicit import for ObjectId class
3. **JAR Rebuild:** Recreate JAR with proper package declarations
4. **SWIG Configuration:** Review SWIG interface file for correct class exposure

**Next Steps:**
- Debug JAR file contents and package structure
- Verify ObjectId class accessibility
- Test alternative compilation approaches
- Consider manual class loading if necessary

### Implementation Notes

**Delete Operation:**
- Takes Table and ObjectId parameters
- Returns boolean indicating success/failure
- Removes specific row identified by ObjectId

**Update Operation:**
- Takes Table, ObjectId, and RowSet parameters
- Updates row identified by ObjectId with new data
- Returns boolean indicating success/failure

**Testing Strategy:**
- Implement comprehensive test cases for both operations
- Verify data integrity after operations
- Test error conditions and edge cases
- Validate proper resource cleanup

This documentation will be updated as the delete and update operations are successfully implemented and tested.