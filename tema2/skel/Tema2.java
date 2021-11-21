import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class Tema2 {
    public static final HashMap<String, Vector <HashMap<Integer, Integer>>> mapResult = new HashMap<>();

    public static void main(String[] args) throws IOException, InterruptedException {
        if (args.length < 3) {
            System.err.println("Usage: Tema2 <workers> <in_file> <out_file>");
            return;
        }
        int NUMBER_OF_THREADS = Integer.parseInt(args[0]);

        File fileIn = new File(args[1]);
        File fileOut = new File(args[2]);

        int bufferSize, docsNr;
        BufferedReader br = new BufferedReader(new FileReader(fileIn));
        BufferedWriter bw = new BufferedWriter(new FileWriter(fileOut));

        bufferSize = Integer.parseInt(br.readLine());
        docsNr = Integer.parseInt(br.readLine());

        ThreadPoolExecutor executor = (ThreadPoolExecutor) Executors.newFixedThreadPool(NUMBER_OF_THREADS);
        String[] files = new String[docsNr];

        Collection<Future<?>> futures = new LinkedList<Future<?>>();


        for (int i = 0; i < docsNr; i++) {
            files[i] = br.readLine();
            mapResult.put(files[i], new Vector<HashMap<Integer, Integer>>());
            int fileSize = (int)(new File(files[i])).length();

            int tasksNr = fileSize / bufferSize;

            for (int j = 0 ; j < tasksNr; j++) {
                futures.add(executor.submit(new TaskMap(files[i], bufferSize * j, bufferSize)));
            }

            if (fileSize % bufferSize != 0) {
                futures.add(executor.submit(new TaskMap(files[i], tasksNr * bufferSize, fileSize % bufferSize)));
            }
        }

        for (Future<?> future:futures) {
            try {
                future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
        executor.shutdown();
        System.out.println("got here");

    }
}
