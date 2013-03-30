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
}
