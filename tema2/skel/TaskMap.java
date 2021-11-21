import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.StringTokenizer;

public class TaskMap implements Runnable {
    final String fileName;
    final int offset;
    final int bufferSize;
    final String delim;

    public TaskMap(String fileName, int offset, int bufferSize) {
        this.fileName = fileName;
        this.offset = offset;
        this.bufferSize = bufferSize;
        delim = " ;:/?˜\\.,><‘\\[\\]\\{\\}\\(\\)!@#$%ˆ&- +’=*”|\r\n\t";
    }

    @Override
    public void run() {
        byte[] array =  new byte[bufferSize];
        int pointerPos = 0;
        try {
            File file = new File(fileName);
            RandomAccessFile randFile = new RandomAccessFile(file, "r");
            randFile.seek(offset);
            randFile.read(array);
            String fileFragment = new String(array);
            if (offset != 0) {
                randFile.seek(offset - 1);
                if (!delim.contains("" + (char)randFile.readByte()) && !delim.contains("" + (char)array[0])) {
                    pointerPos++;
                    while (true) {
                        if (offset + pointerPos + 1 > file.length()) {
                            break;
                        }
                        if (delim.contains("" + (char)array[pointerPos])) {
                            break;
                        }
                        pointerPos++;
                    }
                }
            }

            fileFragment = fileFragment.substring(pointerPos);

            pointerPos = offset + bufferSize - 1;
            randFile.seek(pointerPos);
            String newChar = "" + (char)randFile.read();
            while(true) {
                if (pointerPos + 1 > file.length()) {
                    break;
                }
                if (delim.contains(newChar)) {
                    break;
                }
                newChar = "" + (char)randFile.read();
                fileFragment = fileFragment + newChar;
                pointerPos++;
            }

            HashMap<Integer, Integer> localMap = new HashMap<>();

            StringTokenizer splitFragment = new StringTokenizer(fileFragment, delim);

            while(splitFragment.hasMoreTokens())  {
                String elem = splitFragment.nextToken().toLowerCase();
                if (!elem.equals("")) {

                    int len = elem.length();

                    if (localMap.containsKey(len)) {
                        localMap.put(len, localMap.get(len) + 1);
                    } else {
                        localMap.put(len, 1);
                    }
                }
            }

            synchronized (Tema2.mapResult) {
                Tema2.mapResult.get(fileName).add(localMap);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }


    }
}
