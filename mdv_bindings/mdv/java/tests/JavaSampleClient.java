import javax.print.PrintService;
import javax.sound.sampled.SourceDataLine;
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

import mdv.*;

class JavaSampleClient {

    // Class to hold expected row data for comparison
    static class ExpectedRow {
        String col1;
        int[] col2;
        boolean col3;

        ExpectedRow(String col1, int[] col2, boolean col3) {
            this.col1 = col1;
            this.col2 = col2;
            this.col3 = col3;
        }
    }

    // Class to hold table creation result (UUID and expected data)
    static class TableCreationResult {
        String tableUUID;
        List<ExpectedRow> expectedRows;

        TableCreationResult(String tableUUID, List<ExpectedRow> expectedRows) {
            this.tableUUID = tableUUID;
            this.expectedRows = expectedRows;
        }
    }

    private static TableCreationResult createTable(Client client) {
        // Create table description
        TableDesc desc = new TableDesc("MyTable");
        desc.addField(FieldType.MDV_FLD_TYPE_CHAR, 0, "Col1");
        desc.addField(FieldType.MDV_FLD_TYPE_INT32, 2, "Col2");
        desc.addField(FieldType.MDV_FLD_TYPE_BOOL, 1, "Col3");

        // Create table
        Table table = client.createTable(desc);
        desc.delete();

        // Show table UUID
        UUID uuid = table.getUUID();
        String tableUUID = uuid.toString();
        System.out.println("Table uuid: " + tableUUID);
        uuid.delete();

        // List to hold expected data for comparison
        List<ExpectedRow> expectedRows = new ArrayList<>();

        // INSERT

        // Create rows set
        RowSet rowset = new RowSet(table);

        // Fill rows set
        {
            Row row = new Row(3);                                       // Create row

            // Row 1
            {
                row.setString(0, "Hello");                              // First field is string

                // Second field is pair of integers
                ArrayOfInt32 i32arr = new ArrayOfInt32(2);
                i32arr.set(0, 42);
                i32arr.set(1, 43);
                row.setInt32Array(1, i32arr, 2);
                i32arr.delete();

                row.setBool(2, true);                                   // Third field is boolean

                if (!rowset.add(row))                                   // Add row to rows set
                    System.out.println("Row insertion failed");
                
                // Add expected data for row 1
                expectedRows.add(new ExpectedRow("Hello", new int[]{42, 43}, true));
            }

            // Row 2
            {
                row.setString(0, "World");                              // First field is string
                row.setInt32(1, 44);                                    // Second field is single integer
                row.setBool(2, false);                                  // Third field is boolean
                if (!rowset.add(row))                                   // Add row to rows set
                    System.out.println("Row insertion failed");
                
                // Add expected data for row 2
                expectedRows.add(new ExpectedRow("World", new int[]{44}, false));
            }

            row.delete();                                               // Delete row
        }

        if (!client.insert(rowset))                                     // Insert rows set into the table
            System.out.println("Row insertion into the table failed");

        rowset.delete();                                                // Delete rows set

        table.delete();                                                 // Close table

        return new TableCreationResult(tableUUID, expectedRows);
    }

    private static void selectFromTable(Client client, String tableUUID, List<ExpectedRow> expectedRows) {
        UUID uuid = new UUID(tableUUID);

        Table table = client.getTable(uuid);

        uuid.delete();

        // SELECT

        BitSet bitset = new BitSet(3);                                  // Create bit set
        bitset.fill(true);

        RowSet result = client.select(table, bitset, "");               // Request table content

        if (result == null) {
            System.out.println("Table content reading failed");
            return;
        }

        RowSetEnumerator it = result.enumerator();                      // Get result set iterator

        int rowIndex = 0;
        boolean allMatch = true;

        while(it.next())
        {
            Row row = it.current();

            // Get row ID from the enumerator - direct method call
            ObjectId objId = it.row_id();
            String rowId = "";
            String idOnly = "";
            
            if (objId != null) {
                // Full ObjectId string representation
                rowId = objId.toString();

                // Extract just the ID part (last 16 hex characters = 8 bytes)
                if (rowId.length() >= 16) {
                    idOnly = rowId.substring(rowId.length() - 16);
                } else {
                    idOnly = rowId;
                }
            } else {
                rowId = "null-objid";
                idOnly = "null";
            }

            long f0size = row.fieldSize(0);                             // Get items count in 0's field
            long f1size = row.fieldSize(1);                             // Get items count in 1'st field
            long f2size = row.fieldSize(2);                             // Get items count in 2'nd field

            String f0 = row.getString(0);                               // Get 0's field data
            ArrayOfInt32 f1 = row.getInt32Array(1);                     // Get 1'st field data
            boolean f2 = row.getBool(2);                                // Get 2'nd field data

            // Convert ArrayOfInt32 to int[] for comparison
            int[] col2Array = new int[(int)f1size];
            for (int i = 0; i < f1size; i++) {
                col2Array[i] = f1.get(i);
            }

            // Get expected row
            if (rowIndex < expectedRows.size()) {
                ExpectedRow expected = expectedRows.get(rowIndex);
                
                // Compare each field
                boolean col1Match = f0.equals(expected.col1);
                boolean col2Match = Arrays.equals(col2Array, expected.col2);
                boolean col3Match = (f2 == expected.col3);
                
                boolean rowMatch = col1Match && col2Match && col3Match;
                
                // Log comparison results
                System.out.println("Row " + (rowIndex + 1) + " (ID: " + rowId + ", id-only: " + idOnly + ") comparison:");
                System.out.println("  Col1 - Expected: '" + expected.col1 + "', Actual: '" + f0 + "', Match: " + col1Match);
                System.out.println("  Col2 - Expected: " + Arrays.toString(expected.col2) + ", Actual: " + Arrays.toString(col2Array) + ", Match: " + col2Match);
                System.out.println("  Col3 - Expected: " + expected.col3 + ", Actual: " + f2 + ", Match: " + col3Match);
                System.out.println("  Overall row match: " + rowMatch);
                
                if (!rowMatch) {
                    allMatch = false;
                }
            } else {
                System.out.println("Unexpected row found at index " + rowIndex + " (ID: " + rowId + ", id-only: " + idOnly + ")");
                allMatch = false;
            }

            row.delete();
            if (objId != null) {
                objId.delete();
            }
            rowIndex++;
        }

        // Check if all expected rows were found
        if (rowIndex < expectedRows.size()) {
            System.out.println("Missing rows: expected " + expectedRows.size() + " rows, but found " + rowIndex);
            allMatch = false;
        }

        // Log overall result
        if (allMatch) {
            System.out.println("SUCCESS: All data matches between written and read values!");
        } else {
            System.out.println("FAILURE: Data mismatch between written and read values!");
        }

        it.delete();                                                    // Delete result set iterator

        result.delete();                                                // Delete result set
        bitset.delete();                                                // Delete bit set
        table.delete();
    }

    private static void testDeleteOperation(Client client, String tableUUID, List<ExpectedRow> expectedRows) {
        System.out.println("\n=== Testing DELETE Operation ===");

        UUID uuid = new UUID(tableUUID);
        Table table = client.getTable(uuid);
        uuid.delete();

        if (expectedRows.size() > 0) {
            // Get the first row's ObjectId for deletion
            try {
                // Select rows to get their ObjectIds
                BitSet bitset = new BitSet(3);
                bitset.fill(true);
                RowSet result = client.select(table, bitset, "");

                if (result != null) {
                    RowSetEnumerator it = result.enumerator();

                    if (it.next()) {
                        Row row = it.current();

                        // Get ObjectId from enumerator
                        ObjectId objId = it.row_id();

                        if (objId != null) {
                            System.out.println("Attempting to delete row with ObjectId: " + objId.toString());

                            // Test delete operation
                            boolean deleteResult = client.delete(table, objId);
                            System.out.println("Delete operation result: " + deleteResult);

                            if (deleteResult) {
                                System.out.println("SUCCESS: Row deleted successfully!");
                            } else {
                                System.out.println("FAILED: Row deletion failed!");
                            }
                        } else {
                            System.out.println("Could not obtain ObjectId for deletion test");
                        }

                        row.delete();
                        if (objId != null) {
                            objId.delete();
                        }
                    }

                    it.delete();
                    result.delete();
                }

                bitset.delete();
            } catch (Exception e) {
                System.out.println("Delete test failed with exception: " + e.getMessage());
            }
        }

        table.delete();
    }

    private static void testUpdateOperation(Client client, String tableUUID, List<ExpectedRow> expectedRows) {
        System.out.println("\n=== Testing UPDATE Operation ===");

        UUID uuid = new UUID(tableUUID);
        Table table = client.getTable(uuid);
        uuid.delete();

        if (expectedRows.size() > 0) {
            try {
                // Select rows to get their ObjectIds
                BitSet bitset = new BitSet(3);
                bitset.fill(true);
                RowSet result = client.select(table, bitset, "");

                if (result != null) {
                    RowSetEnumerator it = result.enumerator();

                    if (it.next()) {
                        Row row = it.current();

                        // Get ObjectId from enumerator
                        ObjectId objId = it.row_id();

                        if (objId != null) {
                            System.out.println("Attempting to update row with ObjectId: " + objId.toString());

                            // Create updated row data
                            RowSet updateRowset = new RowSet(table);
                            Row updateRow = new Row(3);

                            // Update with new values
                            updateRow.setString(0, "Updated Hello");
                            ArrayOfInt32 i32arr = new ArrayOfInt32(2);
                            i32arr.set(0, 99);
                            i32arr.set(1, 100);
                            updateRow.setInt32Array(1, i32arr, 2);
                            i32arr.delete();
                            updateRow.setBool(2, false);  // Changed from true to false

                            updateRowset.add(updateRow);
                            updateRow.delete();

                            // Test update operation
                            boolean updateResult = client.update(table, objId, updateRowset);
                            System.out.println("Update operation result: " + updateResult);

                            if (updateResult) {
                                System.out.println("SUCCESS: Row updated successfully!");
                                System.out.println("Updated values: 'Updated Hello', [99, 100], false");
                            } else {
                                System.out.println("FAILED: Row update failed!");
                            }

                            updateRowset.delete();
                        } else {
                            System.out.println("Could not obtain ObjectId for update test");
                        }

                        row.delete();
                        if (objId != null) {
                            objId.delete();
                        }
                    }

                    it.delete();
                    result.delete();
                }

                bitset.delete();
            } catch (Exception e) {
                System.out.println("Update test failed with exception: " + e.getMessage());
            }
        }

        table.delete();
    }

    public static void main(String[] args) {
        System.loadLibrary("mdv4j");

        mdv.clientInitialize();

        // Connect to DB
        Client client = Client.connect(new ClientConfig("tcp://localhost:4800"));   // Connect to DB

        TableCreationResult result = createTable(client);
        String tableUUID = result.tableUUID;
        List<ExpectedRow> expectedRows = result.expectedRows;

        selectFromTable(client, tableUUID, expectedRows);

        // Test the new delete and update operations
        testDeleteOperation(client, tableUUID, expectedRows);
        testUpdateOperation(client, tableUUID, expectedRows);

        client.close();                                                             // Close client

        mdv.clientFinalize();
    }
}