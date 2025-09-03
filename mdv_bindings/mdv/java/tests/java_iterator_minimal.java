import mdv.*;

public class java_iterator_minimal {
    
    public static void main(String[] args) {
        
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");
        
        System.out.println("=== Minimal Iterator Test ===");

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
        
        // Setup test table with unique name to avoid conflicts
        TableDesc tableDesc = new TableDesc("minimal_test_" + System.currentTimeMillis());
        tableDesc.addField(FieldType.MDV_FLD_TYPE_UINT32, 1, "id");
        tableDesc.addField(FieldType.MDV_FLD_TYPE_CHAR, 10, "name");
        
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
        
        // Insert ONE test row
        RowSet insertRowset = new RowSet(table);
        Row r = new Row(2);
        r.setUint32(0, 1);
        r.setString(1, "Test");
        
        if (insertRowset.add(r)) {
            System.out.println("Row added to rowset");
        } else {
            System.out.println("Failed to add row to rowset");
        }
        r.delete();
        
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
        
        // Try to select - but don't iterate yet
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
                
                // Try to call next() - this is where the crash happens
                System.out.println("About to call it.next()...");
                try {
                    boolean hasNext = it.next();
                    System.out.println("it.next() returned: " + hasNext);
                    
                    if (hasNext) {
                        System.out.println("Getting current row...");
                        Row row = it.current();
                        if (row != null) {
                            System.out.println("Got row successfully");
                            row.delete();
                        } else {
                            System.out.println("Row is null");
                        }
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