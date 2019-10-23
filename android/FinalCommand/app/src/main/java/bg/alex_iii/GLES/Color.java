package bg.alex_iii.GLES;

public class Color {
	public static final byte[] WHITE = get(255, 255, 255, 255);
	public static final byte[] BLACK = get(0, 0, 0, 255);
	public static final byte[] RED = get(255, 0, 0, 255);
	public static final byte[] GREEN = get(0, 255, 0, 255);
	public static final byte[] BLUE = get(0, 0, 255, 255);
	public static final byte[] YELLOW = get(255, 255, 0, 255);
	public static final byte[] PURPLE = get(255, 0, 255, 255);
	public static final byte[] CYAN = get(0, 255, 255, 255);
	
	public static byte[] get(int r, int g, int b, int a) {
		byte[] color = { (byte) r, (byte) g, (byte) b, (byte) a };
		return color;
	}
	
	public static byte[] get(float r, float g, float b, float a) {
		return get((int) (r * 255), (int) (g * 255), (int) (b * 255), (int) (a * 255));
	}
}
