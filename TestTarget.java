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
        Thread.sleep(300000);
    }
}