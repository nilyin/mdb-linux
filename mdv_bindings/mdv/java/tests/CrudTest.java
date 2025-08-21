import mdv.*;

public class CrudTest {

    public static void main(String[] args) {
        System.loadLibrary("mdv4j");

        mdv.mdv_initialize();

        Client client = new Client(new ClientConfig());

        Table table = new Table(
            new TableDesc(
                "users",
                new Fields(
                    new Field("name", new CHAR(64)),
                    new Field("age", new UINT32())
                )
            )
        );

        // Create
        try (RowSet insertRowset = new RowSet(table)) {
            insertRowset.add(
                new Datums(
                    new Datum("John Doe"),
                    new Datum(42)
                )
            );
            assert client.insert(insertRowset) == err.MDV_OK;
        }

        // Read
        objid rowId;
        try (RowSet selectRowset = client.select(table, "", null)) {
            try (RowSetEnumerator enumerator = selectRowset.get_enumerator()) {
                assert enumerator.hasNext();
                rowId = enumerator.row_id();
            }
        }

        // Update
        try (RowSet updateRowset = new RowSet(table)) {
            updateRowset.add(
                new Datums(
                    new Datum("Jane Doe"),
                    new Datum(43)
                )
            );
            assert table.update(client, rowId, updateRowset) == err.MDV_OK;
        }

        // Delete
        assert table.delete(client, rowId) == err.MDV_OK;

        mdv.mdv_finalize();
    }
}