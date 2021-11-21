import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

public class TaskReduce implements Runnable {
    final String fileName;
    final Vector<HashMap<Integer, Integer>> vectorElems;

    public TaskReduce(String fileName, Vector<HashMap<Integer, Integer>> vectorElems) {
        this.fileName = fileName;
        this.vectorElems = vectorElems;
    }

    // basic fib formula from the internet
    static int fib(int n) {
        double phi = (1 + Math.sqrt(5)) / 2;
        return (int) Math.round(Math.pow(phi, n)
                / Math.sqrt(5));
    }

    @Override
    public void run() {
        HashMap<Integer, Integer> finalMap = new HashMap<>();
        for (HashMap<Integer, Integer> elem : vectorElems) {
            for (Map.Entry<Integer, Integer> entry : elem.entrySet()) {
                if (finalMap.containsKey(entry.getKey())) {
                    finalMap.put(entry.getKey(), finalMap.get(entry.getKey()) + entry.getValue());
                } else {
                    finalMap.put(entry.getKey(), entry.getValue());
                }
            }
        }


        float rang = 0;
        long sum = 0;
        int maxLen = Integer.MIN_VALUE;
        int wordsNrMax = 0;
        int wordsNr = 0;


        for (Map.Entry<Integer, Integer> elem : finalMap.entrySet()) {
            sum = sum + fib(elem.getKey() + 1) * elem.getValue();
            wordsNr = wordsNr + elem.getValue();
            if (maxLen < elem.getKey()) {
                maxLen = elem.getKey();
                wordsNrMax = elem.getValue();
            }
        }

        rang = (float)sum / (float)wordsNr;

        FinalResult finalResult = new FinalResult(fileName, rang, maxLen, wordsNrMax);
        synchronized(Tema2.reduceResult) {
            Tema2.reduceResult.put(fileName, finalResult);
        }
    }
}
