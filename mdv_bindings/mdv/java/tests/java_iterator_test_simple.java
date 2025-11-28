import mdv.*;

public class java_iterator_test_simple {
    
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
        
        // Setup test table with unique name to avoid conflicts
        TableDesc tableDesc = new TableDesc("iterator_test_" + System.currentTimeMillis());
        tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 32, "name");
        
        // Create the table
        Table table = client.createTable(tableDesc);
        tableDesc.delete();
        
        // Insert test data
        setupTestData(client, table);
        
        // Run basic iterator test
        testBasicIteration(client, table);
        
        // Clean up
        table.delete();
        client.close();
        mdv.clientFinalize();
        
        // Output results
        results.append("\n=== Test Results ===\n");
        results.append("Tests Passed: ").append(testsPassed).append("\n");
        results.append("Tests Failed: ").append(testsFailed).append("\n");
        
        System.out.println(results.toString());
        
        if (testsFailed > 0) {
            System.exit(1);
        }
    }
    
    private static void setupTestData(Client client, Table table) {
        RowSet insertRowset = new RowSet(table);
        try {
            for (int i = 1; i <= 3; i++) {
                Row r = new Row(2);  // 2 fields only
                r.setUint32(0, i);
                r.setString(1, "Item" + i);
                if (!insertRowset.add(r)) {
                    System.out.println("Failed to add row " + i);
                }
                r.delete();
            }
            boolean insertResult = client.insert(insertRowset);
            System.out.println("Insert result: " + insertResult);
        } finally {
            insertRowset.delete();
        }
    }
    
    private static void testBasicIteration(Client client, Table table) {
        String testName = "Basic iteration";
        try {
            int rowCount = 0;
            // Use BitSet with correct size for 2 fields
            BitSet bitSet = new BitSet(2);
            bitSet.fill(true);
            RowSet selectRowset = client.select(table, bitSet, "");
            
            System.out.println("Select result: " + (selectRowset != null ? "success" : "null"));
            
            if (selectRowset != null) {
                RowSetEnumerator it = selectRowset.enumerator();
                try {
                    System.out.println("Starting iteration...");
                    while (it.next()) {
                        System.out.println("Processing row " + (rowCount + 1));
                        Row row = it.current();
                        if (row != null) {
                            rowCount++;
                            System.out.println("Row " + rowCount + " - ID: " + row.getUint32(0) + ", Name: " + row.getString(1));
                            row.delete();
                        } else {
                            System.out.println("Row is null");
                        }
                    }
                    System.out.println("Iteration completed. Total rows: " + rowCount);
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
            e.printStackTrace();
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