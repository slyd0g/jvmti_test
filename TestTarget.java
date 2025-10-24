public class TestTarget {
    
    private static final int[] encoded = {89, 70, 83, 78, 26, 77};
    private static final int key = 42;
    
    public String GetSecret() {
        // Decode the XOR-encoded bytes back to string
        StringBuilder result = new StringBuilder();
        for (int b : encoded) {
            result.append((char)(b ^ key));
        }
        return result.toString();
    }
    
    public static void main(String[] args) throws InterruptedException {
        System.out.println("TestTarget running - PID: " + ProcessHandle.current().pid());
        TestTarget target = new TestTarget();
        
        // Test GetSecret method periodically to see if it gets intercepted
        for (int i = 0; i < 10; i++) {
            System.out.println("Calling GetSecret() - Attempt " + (i + 1));
            String secret = target.GetSecret();
            System.out.println("GetSecret() returned: '" + secret + "'");
            Thread.sleep(2000);
        }
        
        System.out.println("TestTarget finished testing GetSecret()");
        Thread.sleep(300000);
    }
}