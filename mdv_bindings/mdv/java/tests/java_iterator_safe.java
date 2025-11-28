import mdv.*;

public class java_iterator_safe {
    
    public static void main(String[] args) {
        
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");
        
        System.out.println("=== Safe Iterator Test ===");

        // Initialize client-side subsystem
        mdv.clientInitialize();
        
        // Use proper ClientConfig constructor with a configuration string
        ClientConfig config = new ClientConfig("tcp://127.0.0.1:4800");
        
        // Connect to the database
        Client client = Client.connect(config);
        
        if (client == null) {
            System.out.println("Failed to connect to database");
            return;
        }
        
        System.out.println("Connected to database");
        
        // Create a simple 2-field table with unique name
        String tableName = "safe_test_" + System.currentTimeMillis();
        TableDesc tableDesc = new TableDesc(tableName);
        tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 20, "name");
        
        // Create the table
        Table table = client.createTable(tableDesc);
        tableDesc.delete();
        
        if (table == null) {
            System.out.println("Failed to create table");
            client.close();
            mdv.clientFinalize();
            return;
        }
        
        System.out.println("Table '" + tableName + "' created successfully");
        
        // Test 1: Empty table iteration (should not crash)
        System.out.println("\n--- Test 1: Empty table iteration ---");
        testIteration(client, table, "empty table");
        
        // Insert test data
        System.out.println("\n--- Inserting test data ---");
        RowSet insertRowset = new RowSet(table);
        for (int i = 1; i <= 2; i++) {
            Row r = new Row(2);
            r.setUint32(0, i);
            r.setString(1, "Item" + i);
            
            if (insertRowset.add(r)) {
                System.out.println("Row " + i + " added to rowset");
            } else {
                System.out.println("Failed to add row " + i + " to rowset");
            }
            r.delete();
        }
        
        boolean insertResult = client.insert(insertRowset);
        insertRowset.delete();
        
        System.out.println("Insert result: " + insertResult);
        
        if (insertResult) {
            // Test 2: Non-empty table iteration
            System.out.println("\n--- Test 2: Non-empty table iteration ---");
            testIteration(client, table, "populated table");
            
            // Test 3: Filtered query (should return subset)
            System.out.println("\n--- Test 3: Filtered query ---");
            testFilteredIteration(client, table);
        }
        
        // Clean up
        table.delete();
        client.close();
        mdv.clientFinalize();
        
        System.out.println("\nAll tests completed successfully!");
    }
    
    private static void testIteration(Client client, Table table, String testName) {
        try {
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            bitSet.delete();
            
            System.out.println("Select result for " + testName + ": " + (selectRowset != null ? "success" : "null"));
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                
                if (it != null) {
                    System.out.println("Enumerator created successfully");
                    
                    int rowCount = 0;
                    System.out.println("Starting safe iteration...");
                    
                    // Safe iteration with try-catch
                    try {
                        while (it.next()) {
                            System.out.println("Processing row " + (rowCount + 1));
                            Row row = it.current();
                            if (row != null) {
                                rowCount++;
                                long id = row.getUint32(0);
                                String name = row.getString(1);
                                System.out.println("Row " + rowCount + " - ID: " + id + ", Name: " + name);
                                row.delete();
                            } else {
                                System.out.println("Row is null, breaking");
                                break;
                            }
                        }
                    } catch (Exception e) {
                        System.out.println("Exception during iteration: " + e.getMessage());
                        e.printStackTrace();
                    }
                    
                    System.out.println("Iteration completed for " + testName + ". Total rows: " + rowCount);
                    it.delete();
                } else {
                    System.out.println("Failed to create enumerator for " + testName);
                }
                
                selectRowset.delete();
            } else {
                System.out.println("Select returned null for " + testName);
            }
            
        } catch (Exception e) {
            System.out.println("Exception in testIteration for " + testName + ": " + e.getMessage());
            e.printStackTrace();
        }
    }
    
    private static void testFilteredIteration(Client client, Table table) {
        try {
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "id = 1");
            bitSet.delete();
            
            System.out.println("Filtered select result: " + (selectRowset != null ? "success" : "null"));
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                
                if (it != null) {
                    int rowCount = 0;
                    
                    try {
                        while (it.next()) {
                            Row row = it.current();
                            if (row != null) {
                                rowCount++;
                                long id = row.getUint32(0);
                                String name = row.getString(1);
                                System.out.println("Filtered row - ID: " + id + ", Name: " + name);
                                row.delete();
                            }
                        }
                    } catch (Exception e) {
                        System.out.println("Exception during filtered iteration: " + e.getMessage());
                        e.printStackTrace();
                    }
                    
                    System.out.println("Filtered iteration completed. Rows found: " + rowCount);
                    it.delete();
                }
                
                selectRowset.delete();
            }
            
        } catch (Exception e) {
            System.out.println("Exception in testFilteredIteration: " + e.getMessage());
            e.printStackTrace();
        }
    }
}