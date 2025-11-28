import mdv.*;

public class JarTest {
    public static void main(String[] args) {
        System.out.println("=== SWIG Java Bindings Test ===");

        try {
            // Load native JNI library and initialize
            System.loadLibrary("mdv4j");
            mdv.clientInitialize();

            // Verify core binding classes are present
            System.out.println("✅ Client class: " + Client.class.getName());
            System.out.println("✅ Table class: " + Table.class.getName());
            System.out.println("✅ RowSet class: " + RowSet.class.getName());
            System.out.println("✅ RowSetEnumerator class: " + RowSetEnumerator.class.getName());
            System.out.println("✅ ObjectId class: " + ObjectId.class.getName());
            System.out.println("✅ BitSet class: " + BitSet.class.getName());
            System.out.println("✅ UUID class: " + UUID.class.getName());
            System.out.println("✅ Row class: " + Row.class.getName());

            // FieldType enum sanity check
            System.out.println("✅ FieldType constants available: " + FieldType.MDV_FLD_TYPE_UINT32);

            // Finalize client-side subsystem
            mdv.clientFinalize();

            System.out.println("\n✅ SUCCESS: All SWIG Java classes loaded successfully!");
        } catch (Throwable e) {
            System.out.println("❌ FAIL: " + e.getMessage());
            e.printStackTrace();
        }
    }
}
