import java.io.*;
import java.util.*;
import java.util.concurrent.*;

public class Tema2 {
    public static final HashMap<String, Vector <HashMap<Integer, Integer>>> mapResult = new HashMap<>();
    public static final HashMap<String, FinalResult> reduceResult = new HashMap<>();

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


        // preparing map
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

        // running map
        for (Future<?> future:futures) {
            try {
                future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
        executor.shutdown();

        // preparing reduce
        executor = (ThreadPoolExecutor) Executors.newFixedThreadPool(NUMBER_OF_THREADS);
        futures = new LinkedList<Future<?>>();

        for (Map.Entry<String, Vector <HashMap<Integer, Integer>>> fileElem : mapResult.entrySet()) {
            futures.add(executor.submit(new TaskReduce(fileElem.getKey(), fileElem.getValue())));
        }

        // running reduce
        for (Future<?> future:futures) {
            try {
                future.get();
            } catch (InterruptedException | ExecutionException e) {
                e.printStackTrace();
            }
        }
        executor.shutdown();

        Map<String, FinalResult> map = new TreeMap<>(reduceResult);


        for (Map.Entry<String, FinalResult> entry : map.entrySet()) {
            bw.write(entry.getValue().toString() + "\n");
        }
        bw.flush();
        bw.close();
    }
}
