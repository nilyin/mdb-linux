import mdv.*;

public class java_iterator_clean {
    
    public static void main(String[] args) {
        
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");
        
        System.out.println("=== Clean Database Iterator Test ===");

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
        
        // Create a simple 2-field table
        TableDesc tableDesc = new TableDesc("clean_test");
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
        
        System.out.println("Table created successfully");
        
        // Insert test data
        RowSet insertRowset = new RowSet(table);
        for (int i = 1; i <= 3; i++) {
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
        
        if (!insertResult) {
            System.out.println("Insert failed, exiting");
            table.delete();
            client.close();
            mdv.clientFinalize();
            return;
        }
        
        // Now try to select and iterate
        BitSet bitSet = new BitSet(2);
        bitSet.fill(true);
        RowSet selectRowset = client.select(table, bitSet, "");
        bitSet.delete();
        
        System.out.println("Select result: " + (selectRowset != null ? "success" : "null"));
        
        if (selectRowset != null) {
            System.out.println("Getting enumerator...");
            RowSetEnumerator it = selectRowset.enumerator();
            
            if (it != null) {
                System.out.println("Enumerator created successfully");
                
                int rowCount = 0;
                System.out.println("Starting iteration...");
                
                try {
                    while (it.next()) {
                        System.out.println("Processing row " + (rowCount + 1));
                        Row row = it.current();
                        if (row != null) {
                            rowCount++;
                            int id = row.getUint32(0);
                            String name = row.getString(1);
                            System.out.println("Row " + rowCount + " - ID: " + id + ", Name: " + name);
                            row.delete();
                        } else {
                            System.out.println("Row is null");
                        }
                    }
                    System.out.println("Iteration completed. Total rows: " + rowCount);
                    
                    if (rowCount == 3) {
                        System.out.println("✅ SUCCESS: All 3 rows processed correctly");
                    } else {
                        System.out.println("❌ FAILURE: Expected 3 rows, got " + rowCount);
                    }
                    
                } catch (Exception e) {
                    System.out.println("Exception during iteration: " + e.getMessage());
                    e.printStackTrace();
                }
                
                it.delete();
            } else {
                System.out.println("Failed to create enumerator");
            }
            
            selectRowset.delete();
        }
        
        // Clean up
        table.delete();
        client.close();
        mdv.clientFinalize();
        
        System.out.println("Test completed");
    }
}