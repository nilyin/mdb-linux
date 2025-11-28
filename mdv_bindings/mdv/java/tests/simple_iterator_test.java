import mdv.*;

public class simple_iterator_test {

    public static void main(String[] args) {
        System.out.println("=== Simple Java Iterator Test ===");

        try {
            System.loadLibrary("mdv4j");
            System.out.println("✅ PASS: Library loaded successfully");
        } catch (UnsatisfiedLinkError e) {
            System.out.println("❌ FAIL: Library load failed - " + e.getMessage());
            return;
        }

        try {
            mdv.clientInitialize();
            System.out.println("✅ PASS: mdv.clientInitialize()");

            Client client = Client.connect(new ClientConfig("tcp://localhost:4800"));
            if (client != null) {
                System.out.println("✅ PASS: Client.connect succeeded");
                client.close();
            } else {
                System.out.println("❌ FAIL: Client.connect returned null");
            }

            mdv.clientFinalize();
        } catch (Exception e) {
            System.out.println("❌ FAIL: Unexpected error - " + e.getMessage());
        }
    }
}