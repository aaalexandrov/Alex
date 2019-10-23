package bg.alex_iii.GLES;

public class Util {
	public static final float EPS = 0.0001f;
	
	public static float sqr(float a) {
		return a * a;
	}
	
	public static boolean isZero(float a) {
		if (a < 0)
			return a >= -EPS;
		else
			return a <= EPS;
	}
	
	public static boolean isEqual(float a, float b) {
		if (a > b)
			return a - b <= EPS;
		else
			return b - a <= EPS;
	}
	
	public static int roundToPowerOf2(int i) {
		--i;
		i |= i >> 1;
		i |= i >> 2;
		i |= i >> 4;
		i |= i >> 8;
		i |= i >> 16;
		++i;
		return i;
	}
	
	public static int powerOf2Log2(int i) {
		int log = 0;
		if ((i & 0xaaaaaaaa) != 0)
			log += 1;
		if ((i & 0xcccccccc) != 0)
			log += 2;
		if ((i & 0xf0f0f0f0) != 0)
			log += 4;
		if ((i & 0xff00ff00) != 0)
			log += 8;
		if ((i & 0xffff0000) != 0)
			log += 16;
		return log;
	}
	
	public static int clamp(int x, int min, int max) {
		return Math.min(Math.max(min, x), max);
	}
	
	public static long clamp(long x, long min, long max) {
		return Math.min(Math.max(min, x), max);
	}
	
	public static float clamp(float x, float min, float max) {
		return Math.min(Math.max(min, x), max);
	}
	
	public static double clamp(double x, double min, double max) {
		return Math.min(Math.max(min, x), max);
	}
}
