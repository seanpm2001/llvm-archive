import java.util.*;

public class Maps1
{
    public static Random rand = new Random(0);

    public static void main(String[] args) {
        HashMap map = new HashMap();
        fillMapWithRandomInts(map);
        printIntMap(map);
    }
    }
}
