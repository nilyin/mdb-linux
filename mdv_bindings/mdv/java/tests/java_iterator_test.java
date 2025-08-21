import mdv.*;

public class java_iterator_test {
    
    private static int testsPassed = 0;
    private static int testsFailed = 0;
    private static StringBuilder results = new StringBuilder();
    
    public static void main(String[] args) {
        System.loadLibrary("mdv4j");
        
        results.append("=== Java Iterator Test Suite ===\n");
        
        mdv.mdv_initialize();
        
        Client client = new Client(new ClientConfig());
        
        // Setup test table
        Table table = new Table(
            new TableDesc(
                "iterator_test",
                new Fields(
                    new Field("id", new UINT32()),
                    new Field("name", new CHAR(32)),
                    new Field("value", new INT64())
                )
            )
        );
        
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
        
        mdv.mdv_finalize();
        
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
        try (RowSet insertRowset = new RowSet(table)) {
            for (int i = 1; i <= 5; i++) {
                insertRowset.add(
                    new Datums(
                        new Datum(i),
                        new Datum("Item" + i),
                        new Datum((long)(i * 100))
                    )
                );
            }
            client.insert(insertRowset);
        }
    }
    
    // Test 1: Try-with-resources pattern (recommended)
    private static void testTryWithResources(Client client, Table table) {
        String testName = "Try-with-resources pattern";
        try {
            int rowCount = 0;
            try (RowSet selectRowset = client.select(table, "", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    while (it.hasNext()) {
                        Row row = it.next();
                        rowCount++;
                        row.delete();
                    }
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
            RowSet selectRowset = client.select(table, "", null);
            RowSetEnumerator it = selectRowset.get_enumerator();
            
            try {
                while (it.hasNext()) {
                    Row row = it.next();
                    rowCount++;
                    row.delete();
                }
            } finally {
                it.close();
                selectRowset.delete();
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
    
    // Test 3: Enhanced for-each loop
    private static void testEnhancedForLoop(Client client, Table table) {
        String testName = "Enhanced for-each loop";
        try {
            int rowCount = 0;
            try (RowSet selectRowset = client.select(table, "", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    for (Row row : it) {
                        rowCount++;
                        row.delete();
                    }
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
            try (RowSet selectRowset = client.select(table, "", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    while (it.moveNext()) {  // Old method
                        Row row = it.current();  // Old method
                        rowCount++;
                        row.delete();
                    }
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
            try (RowSet selectRowset = client.select(table, "id > 100", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    while (it.hasNext()) {
                        Row row = it.next();
                        rowCount++;
                        row.delete();
                    }
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
            try (RowSet selectRowset = client.select(table, "id = 1", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    while (it.hasNext()) {
                        Row row = it.next();
                        rowCount++;
                        row.delete();
                    }
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
            try (RowSet selectRowset = client.select(table, "", null)) {
                int count1 = 0, count2 = 0;
                
                try (RowSetEnumerator it1 = selectRowset.get_enumerator()) {
                    while (it1.hasNext()) {
                        Row row = it1.next();
                        count1++;
                        row.delete();
                    }
                }
                
                try (RowSetEnumerator it2 = selectRowset.get_enumerator()) {
                    while (it2.hasNext()) {
                        Row row = it2.next();
                        count2++;
                        row.delete();
                    }
                }
                
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
            try (RowSet selectRowset = client.select(table, "", null)) {
                try (RowSetEnumerator it = selectRowset.get_enumerator()) {
                    while (it.hasNext()) {
                        Row row = it.next();
                        row.delete();
                        // Simulate exception
                        if (row.getUInt32(0) == 3) {
                            throw new RuntimeException("Test exception");
                        }
                    }
                } catch (RuntimeException e) {
                    exceptionCaught = true;
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
            try (RowSet selectRowset = client.select(table, "", null)) {
                it = selectRowset.get_enumerator();
                it.close();
            }
            
            // Try to use closed iterator
            boolean exceptionThrown = false;
            try {
                it.hasNext();
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
            try (RowSet selectRowset = client.select(table, "", null)) {
                RowSetEnumerator it = selectRowset.get_enumerator();
                
                while (it.hasNext() && rowCount < 3) {
                    Row row = it.next();
                    rowCount++;
                    row.delete();
                }
                
                it.close(); // Early close
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