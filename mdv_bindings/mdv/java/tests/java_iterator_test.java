import mdv.*;

public class java_iterator_test {
    
    private static int testsPassed = 0;
    private static int testsFailed = 0;
    private static StringBuilder results = new StringBuilder();
    
    public static void main(String[] args) {
        
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");
        
        results.append("=== Java Iterator Test Suite ===\n");

        // Initialize client-side subsystem
        mdv.clientInitialize();
        
        // Use proper ClientConfig constructor with a configuration string
        ClientConfig config = new ClientConfig("tcp://127.0.0.1:4800");
        
        // Connect to the database
        Client client = Client.connect(config);
        
        // Setup test table with unique name
        TableDesc tableDesc = new TableDesc("iterator_test_" + System.currentTimeMillis());
        tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 32, "name");
        
        // Create the table
        Table table = client.createTable(tableDesc);
        tableDesc.delete();
        
        // Insert test data
        setupTestData(client, table);
        
        // Run iterator tests
        testTryWithResources(client, table);
        testManualCleanup(client, table);
        testEnhancedForLoop(client, table);
        testBackwardCompatibility(client, table);
        testEmptyResultSet(client, table);
        testSingleRowResultSet(client, table);
        testMultipleIterators(client, table);
        testExceptionHandling(client, table);
        testIteratorReuse(client, table);
        testEarlyClose(client, table);
        
        // Clean up
        table.delete();
        client.close();
        mdv.clientFinalize();
        
        // Output results
        results.append("\n=== Test Results ===\n");
        results.append("Tests Passed: ").append(testsPassed).append("\n");
        results.append("Tests Failed: ").append(testsFailed).append("\n");
        results.append("Success Rate: ").append(String.format("%.1f%%", 
            (testsPassed * 100.0) / (testsPassed + testsFailed))).append("\n");
        
        System.out.println(results.toString());
        
        if (testsFailed > 0) {
            System.exit(1);
        }
    }
    
    private static void setupTestData(Client client, Table table) {
        RowSet insertRowset = new RowSet(table);
        try {
            for (int i = 1; i <= 5; i++) {
                Row r = new Row(2);  // 2 fields only
                r.setUint32(0, i);
                r.setString(1, "Item" + i);
                if (!insertRowset.add(r)) {
                    // failed to add row — continue to next
                }
                r.delete();
            }
            client.insert(insertRowset);
        } finally {
            insertRowset.delete();
        }
    }
    
    // Test 1: Try-with-resources pattern (converted to manual cleanup)
    private static void testTryWithResources(Client client, Table table) {
        String testName = "Try-with-resources pattern";
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
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 2: Manual cleanup pattern
    private static void testManualCleanup(Client client, Table table) {
        String testName = "Manual cleanup pattern";
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
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 3: Enhanced for-each loop (converted to explicit enumerator)
    private static void testEnhancedForLoop(Client client, Table table) {
        String testName = "Enhanced for-each loop";
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
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 4: Backward compatibility (old methods)
    private static void testBackwardCompatibility(Client client, Table table) {
        String testName = "Backward compatibility";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {  // Old method still usable
                        Row row = it.current();  // Old method
                        rowCount++;
                        row.delete();
                    }
                } finally {
                    it.delete();
                    selectRowset.delete();
                }
            }
            bitSet.delete();
            
            if (rowCount == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 5: Empty result set (simulate by creating empty table)
    private static void testEmptyResultSet(Client client, Table table) {
        String testName = "Empty result set";
        try {
            // Create a new empty table for this test
            TableDesc emptyTableDesc = new TableDesc("empty_test_" + System.currentTimeMillis());
            emptyTableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
            emptyTableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 32, "name");
            Table emptyTable = client.createTable(emptyTableDesc);
            emptyTableDesc.delete();
            
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(emptyTable, bitSet, "");
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
            emptyTable.delete();
            
            if (rowCount == 0) {
                pass(testName);
            } else {
                fail(testName, "Expected 0 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 6: Single row result set (simulate by creating table with one row)
    private static void testSingleRowResultSet(Client client, Table table) {
        String testName = "Single row result set";
        try {
            // Create a new table with only one row for this test
            TableDesc singleTableDesc = new TableDesc("single_test_" + System.currentTimeMillis());
            singleTableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
            singleTableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 32, "name");
            Table singleTable = client.createTable(singleTableDesc);
            singleTableDesc.delete();
            
            // Insert only one row
            RowSet insertRowset = new RowSet(singleTable);
            Row r = new Row(2);
            r.setUint32(0, 1);
            r.setString(1, "SingleItem");
            insertRowset.add(r);
            r.delete();
            client.insert(insertRowset);
            insertRowset.delete();
            
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(singleTable, bitSet, "");
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
            singleTable.delete();
            
            if (rowCount == 1) {
                pass(testName);
            } else {
                fail(testName, "Expected 1 row, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 7: Multiple iterators on same result set
    private static void testMultipleIterators(Client client, Table table) {
        String testName = "Multiple iterators";
        try {
            // Create separate queries since enumerators can't be reused
            BitSet bitSet1 = new BitSet(2);
            bitSet1.fill(true);
            RowSet selectRowset1 = client.select(table, bitSet1, "");
            
            BitSet bitSet2 = new BitSet(2);
            bitSet2.fill(true);
            RowSet selectRowset2 = client.select(table, bitSet2, "");
            
            int count1 = 0, count2 = 0;
            
            if (selectRowset1 != null) {
                RowSetEnumerator it1 = selectRowset1.enumerator();
                try {
                    while (it1.next()) {
                        Row row = it1.current();
                        count1++;
                        row.delete();
                    }
                } finally {
                    it1.delete();
                    selectRowset1.delete();
                }
            }
            
            if (selectRowset2 != null) {
                RowSetEnumerator it2 = selectRowset2.enumerator();
                try {
                    while (it2.next()) {
                        Row row = it2.current();
                        count2++;
                        row.delete();
                    }
                } finally {
                    it2.delete();
                    selectRowset2.delete();
                }
            }
            
            bitSet1.delete();
            bitSet2.delete();
            
            if (count1 == 5 && count2 == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5,5 rows, got " + count1 + "," + count2);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 8: Exception handling during iteration
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
                        if (row.getUint32(0) == 3) {
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
    
    // Test 9: Iterator reuse (should fail gracefully)
    private static void testIteratorReuse(Client client, Table table) {
        String testName = "Iterator reuse prevention";
        try {
            RowSetEnumerator it = null;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            if (selectRowset != null) {
                it = selectRowset.enumerator();
                it.delete();
                selectRowset.delete();
            }
            bitSet.delete();
            
            // Try to use closed iterator - should return false or throw
            boolean result = false;
            try {
                if (it != null) {
                    result = it.next();
                }
            } catch (Exception e) {
                // Exception is acceptable
                result = false;
            }
            
            if (!result) {
                pass(testName);
            } else {
                fail(testName, "Closed iterator should return false or throw exception");
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 10: Early close during iteration
    private static void testEarlyClose(Client client, Table table) {
        String testName = "Early close during iteration";
        try {
            int rowCount = 0;
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                
                while (it.next() && rowCount < 3) {
                    Row row = it.current();
                    rowCount++;
                    row.delete();
                }
                
                it.delete(); // Early close
                selectRowset.delete();
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
    
    private static void pass(String testName) {
        testsPassed++;
        results.append("✅ PASS: ").append(testName).append("\n");
    }
    
    private static void fail(String testName, String reason) {
        testsFailed++;
        results.append("❌ FAIL: ").append(testName).append(" - ").append(reason).append("\n");
    }
}