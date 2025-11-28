import mdv.*;

public class java_iterator_final {
    
    private static int testsPassed = 0;
    private static int testsFailed = 0;
    private static StringBuilder results = new StringBuilder();
    
    public static void main(String[] args) {
        
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");
        
        results.append("=== Java Iterator Test Suite (Final) ===\n");

        // Initialize client-side subsystem
        mdv.clientInitialize();
        
        try {
            // Use proper ClientConfig constructor with a configuration string
            ClientConfig config = new ClientConfig("tcp://127.0.0.1:4800");
            
            // Connect to the database
            Client client = Client.connect(config);
            
            if (client == null) {
                results.append("❌ CRITICAL: Failed to connect to database server\n");
                results.append("Please ensure MedvedDB server is running and database is clean\n");
                System.out.println(results.toString());
                return;
            }
            
            results.append("✅ Connected to database successfully\n");
            
            // Create a completely new table with unique name
            String uniqueTableName = "test_" + System.currentTimeMillis() + "_" + (int)(Math.random() * 1000);
            results.append("Creating table: " + uniqueTableName + "\n");
            
            TableDesc tableDesc = new TableDesc(uniqueTableName);
            tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
            tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 20, "name");
            
            // Create the table
            Table table = client.createTable(tableDesc);
            tableDesc.delete();
            
            if (table == null) {
                results.append("❌ CRITICAL: Failed to create table\n");
                client.close();
                System.out.println(results.toString());
                return;
            }
            
            results.append("✅ Table created successfully\n");
            
            // Insert fresh test data
            results.append("Inserting test data...\n");
            boolean insertSuccess = setupTestData(client, table);
            
            if (!insertSuccess) {
                results.append("❌ CRITICAL: Failed to insert test data\n");
                table.delete();
                client.close();
                System.out.println(results.toString());
                return;
            }
            
            results.append("✅ Test data inserted successfully\n");
            
            // Run iterator tests
            testBasicIteration(client, table);
            testEmptyResultSet(client, table);
            testSingleRowResultSet(client, table);
            testMultipleIterators(client, table);
            testExceptionHandling(client, table);
            testEarlyClose(client, table);
            
            // Clean up
            table.delete();
            client.close();
            
        } catch (Exception e) {
            results.append("❌ CRITICAL EXCEPTION: " + e.getMessage() + "\n");
            e.printStackTrace();
        } finally {
            mdv.clientFinalize();
        }
        
        // Output results
        results.append("\n=== Test Results ===\n");
        results.append("Tests Passed: ").append(testsPassed).append("\n");
        results.append("Tests Failed: ").append(testsFailed).append("\n");
        if (testsPassed + testsFailed > 0) {
            results.append("Success Rate: ").append(String.format("%.1f%%", 
                (testsPassed * 100.0) / (testsPassed + testsFailed))).append("\n");
        }
        
        System.out.println(results.toString());
        
        if (testsFailed > 0) {
            System.exit(1);
        }
    }
    
    private static boolean setupTestData(Client client, Table table) {
        RowSet insertRowset = new RowSet(table);
        try {
            for (int i = 1; i <= 3; i++) {  // Reduced to 3 rows for simpler testing
                Row r = new Row(2);
                r.setUint32(0, i);
                r.setString(1, "Item" + i);
                
                if (!insertRowset.add(r)) {
                    results.append("❌ Failed to add row " + i + " to rowset\n");
                    r.delete();
                    return false;
                }
                r.delete();
            }
            
            boolean insertResult = client.insert(insertRowset);
            if (!insertResult) {
                results.append("❌ Insert operation failed\n");
                return false;
            }
            
            return true;
        } finally {
            insertRowset.delete();
        }
    }
    
    private static void testBasicIteration(Client client, Table table) {
        String testName = "Basic iteration";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {
                        Row row = it.current();
                        if (row != null) {
                            rowCount++;
                            // Verify we can read the data
                            long id = row.getUint32(0);
                            String name = row.getString(1);
                            results.append("  Row " + rowCount + ": ID=" + id + ", Name=" + name + "\n");
                            row.delete();
                        }
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 3) {
                pass(testName);
            } else {
                fail(testName, "Expected 3 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void testEmptyResultSet(Client client, Table table) {
        String testName = "Empty result set";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "id > 100");
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {
                        Row row = it.current();
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 0) {
                pass(testName);
            } else {
                fail(testName, "Expected 0 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void testSingleRowResultSet(Client client, Table table) {
        String testName = "Single row result set";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "id = 1");
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {
                        Row row = it.current();
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 1) {
                pass(testName);
            } else {
                fail(testName, "Expected 1 row, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void testMultipleIterators(Client client, Table table) {
        String testName = "Multiple iterators";
        try {
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            
            if (selectRowset != null) {
                int count1 = 0, count2 = 0;
                
                RowSetEnumerator it1 = selectRowset.enumerator();
                try {
                    while (it1.next()) {
                        Row row = it1.current();
                        count1++;
                        row.delete();
                    }
                } finally {
                    it1.delete();
                }
                
                RowSetEnumerator it2 = selectRowset.enumerator();
                try {
                    while (it2.next()) {
                        Row row = it2.current();
                        count2++;
                        row.delete();
                    }
                } finally {
                    it2.delete();
                }
                
                selectRowset.delete();
                
                if (count1 == 3 && count2 == 3) {
                    pass(testName);
                } else {
                    fail(testName, "Expected 3,3 rows, got " + count1 + "," + count2);
                }
            }
            bitSet.delete();
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void testExceptionHandling(Client client, Table table) {
        String testName = "Exception handling";
        try {
            boolean exceptionCaught = false;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {
                        Row row = it.current();
                        if (row.getUint32(0) == 2) {  // Throw exception on second row
                            throw new RuntimeException("Test exception");
                        }
                        row.delete();
                    }
                } catch (RuntimeException e) {
                    exceptionCaught = true;
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (exceptionCaught) {
                pass(testName);
            } else {
                fail(testName, "Exception not caught");
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void testEarlyClose(Client client, Table table) {
        String testName = "Early close during iteration";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                
                while (it.next() && rowCount < 2) {  // Stop after 2 rows
                    Row row = it.current();
                    rowCount++;
                    row.delete();
                }
                
                it.delete(); // Early close
                selectRowset.delete();
            }
            bitSet.delete();
            
            if (rowCount == 2) {
                pass(testName);
            } else {
                fail(testName, "Expected 2 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    private static void pass(String testName) {
        testsPassed++;
        results.append("✅ PASS: ").append(testName).append("\n");
    }
    
    private static void fail(String testName, String reason) {
        testsFailed++;
        results.append("❌ FAIL: ").append(testName).append(" - ").append(reason).append("\n");
    }
}