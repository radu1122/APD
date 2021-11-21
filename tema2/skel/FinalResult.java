public class FinalResult {
    final private String fileName;
    final private float rang;
    final private int maxLen;
    final private int wordsNr;

    public FinalResult(String fileName, float rang, int maxLen, int wordsNr) {
        this.fileName = fileName;
        this.rang = rang;
        this.maxLen = maxLen;
        this.wordsNr = wordsNr;
    }

    public String getFileName() {
        return fileName;
    }

    public float getRang() {
        return rang;
    }

    public int getMaxLen() {
        return maxLen;
    }

    public int getWordsNr() {
        return wordsNr;
    }

    @Override
    public String toString() {
        return fileName + ", " + String.format("%.2f", rang) + ", " + maxLen + ", " + wordsNr;
    }
}
