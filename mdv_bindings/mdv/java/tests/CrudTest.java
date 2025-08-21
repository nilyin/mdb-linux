import mdv.*;

public class CrudTest {

    public static void main(String[] args) {
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");

        // Initialize client-side subsystem
        mdv.clientInitialize();

        // Connect client (provide DB address)
        Client client = Client.connect(new ClientConfig("tcp://localhost:4800"));

        // Describe table and its fields using the current SWIG API
        TableDesc desc = new TableDesc("users");
        desc.addField(FieldType.MDV_FLD_TYPE_CHAR, 64, "name");
        desc.addField(FieldType.MDV_FLD_TYPE_UINT32, 0, "age");

        // Create the table via the client
        Table table = client.createTable(desc);

        // Insert a row
        RowSet insertRowset = new RowSet(table);
        {
            Row r = new Row(2); // two columns
            r.setString(0, "John Doe");
            r.setUint32(1, 42);
            if (!insertRowset.add(r)) {
                System.out.println("Failed to add row to RowSet");
            }
            r.delete();
        }
        if (!client.insert(insertRowset)) {
            System.out.println("Insert failed");
        }
        insertRowset.delete();

        // Select rows and print the first one
        RowSet selectRowset = client.mdv_client_select_impl(table, (BitSet) null, "");
        if (selectRowset != null) {
            try (RowSetEnumerator enumerator = selectRowset.enumerator()) {
                if (enumerator.hasNext()) {
                    Row row = enumerator.next();
                    String name = row.getString(0);
                    long age = row.getUint32(1);
                    System.out.println("Found row: name=" + name + ", age=" + age);
                    row.delete();
                } else {
                    System.out.println("No rows found");
                }
            }
            selectRowset.delete();
        } else {
            System.out.println("Select returned null");
        }

        // Finalize client-side subsystem
        mdv.clientFinalize();
    }
}