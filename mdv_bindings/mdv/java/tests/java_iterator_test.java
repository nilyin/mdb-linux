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
        
        // Setup test table
        TableDesc tableDesc = new TableDesc("iterator_test");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 32, "name");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_INT64, 1, "value");
        
        // Create the table
        Table table = client.createTable(tableDesc);
        
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
                Row r = new Row(3);
                r.setUint32(0, i);
                r.setString(1, "Item" + i);
                r.setInt64(2, (long)(i * 100));
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            
            if (rowCount == 5) {
                pass(testName);
            } else {
                fail(testName, "Expected 5 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 5: Empty result set
    private static void testEmptyResultSet(Client client, Table table) {
        String testName = "Empty result set";
        try {
            int rowCount = 0;
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            
            if (rowCount == 0) {
                pass(testName);
            } else {
                fail(testName, "Expected 0 rows, got " + rowCount);
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 6: Single row result set
    private static void testSingleRowResultSet(Client client, Table table) {
        String testName = "Single row result set";
        try {
            int rowCount = 0;
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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
                
                if (count1 == 5 && count2 == 5) {
                    pass(testName);
                } else {
                    fail(testName, "Expected 5,5 rows, got " + count1 + "," + count2);
                }
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    while (it.next()) {
                        Row row = it.current();
                        // Use correct method name (getUint32 instead of getUInt32)
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
            
            if (exceptionCaught) {
                pass(testName);
            } else {
                fail(testName, "Exception not caught");
            }
        } catch (Exception e) {
            fail(testName, e.getMessage());
        }
    }
    
    // Test 9: Iterator reuse (should fail)
    private static void testIteratorReuse(Client client, Table table) {
        String testName = "Iterator reuse prevention";
        try {
            RowSetEnumerator it;
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            if (selectRowset != null) {
                it = selectRowset.enumerator();
                it.delete();
                selectRowset.delete();
            } else {
                it = null;
            }
            
            // Try to use closed iterator
            boolean exceptionThrown = false;
            try {
                it.next();
            } catch (Exception e) {
                exceptionThrown = true;
            }
            
            if (exceptionThrown) {
                pass(testName);
            } else {
                fail(testName, "Closed iterator should throw exception");
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
            // Use BitSet instead of String for the second parameter
            BitSet bitSet = new BitSet(0);
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