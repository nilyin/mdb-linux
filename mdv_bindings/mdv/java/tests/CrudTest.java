import mdv.*;

public class CrudTest {

    public static void main(String[] args) {
        // Load native JNI library (libmdv4j.so)
        System.loadLibrary("mdv4j");

        // Initialize client-side subsystem
        mdv.clientInitialize();

        // Connect client
        Client client = Client.connect(new ClientConfig());

        // Describe table and its fields using the current SWIG API
        TableDesc desc = new TableDesc("users");
        desc.addField(FieldType.MDV_FLD_TYPE_CHAR, 64, "name");
        desc.addField(FieldType.MDV_FLD_TYPE_UINT32, 0, "age");

        // Create the table via the client
        Table table = client.createTable(desc);

        // Insert a row
        try (RowSet insertRowset = new RowSet(table)) {
            Row r = new Row(2); // two columns
            r.setString(0, "John Doe");
            r.setUint32(1, 42);
            insertRowset.add(r);

            // Client.insert returns boolean in current bindings
            assert client.insert(insertRowset);
        }

        // Select rows and print the first one
        RowSet selectRowset = client.mdv_client_select_impl(table, null, "");
        try (RowSetEnumerator enumerator = selectRowset.get_enumerator()) {
            if (enumerator.hasNext()) {
                Row row = enumerator.next();
                String name = row.getString(0);
                long age = row.getUint32(1);
                System.out.println("Found row: name=" + name + ", age=" + age);
            } else {
                System.out.println("No rows found");
            }
        }

        // Finalize client-side subsystem
        mdv.clientFinalize();
    }
}