package bg.alex_iii.GLES;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Paint.Align;
import android.graphics.Paint.FontMetrics;
import android.graphics.RectF;
import android.graphics.Typeface;

public class GLESFont {
	public static final String TAG = "GLESFont"; 
	
	public String mName;
	public float mSize;
	public int mStyle;
	public float mCharWidth;
	public char mFirstChar, mLastChar;
	public FontMetrics mFontMetrics;  
	public GLESTexture mTexture;
	public int mCharRows, mCharCols;
	
	public GLESFont(String name, float size, int style) {
		mName = name;
		mSize = size;
		mStyle = style;
	}
	
	public boolean init() {
		Paint paint = new Paint();
		paint.setTypeface(Typeface.create(Typeface.MONOSPACE, mStyle));
		paint.setTextSize(mSize);
		paint.setARGB(255, 255, 255, 255);
		paint.setTextAlign(Align.LEFT);
		paint.setDither(false);
		paint.setSubpixelText(true);
		
		mFontMetrics = paint.getFontMetrics();
		float width, height;
		mCharWidth = (float) Math.ceil(paint.measureText(" "));
		width = mCharWidth;
		height = (float) Math.ceil(mFontMetrics.bottom - mFontMetrics.top);
		mFirstChar = ' ';
		mLastChar = 128;
		int pixels = (int) width * (int) height * (mLastChar - mFirstChar);
		pixels = Util.roundToPowerOf2(pixels);
		int log = Util.powerOf2Log2(pixels);
		int xPow2 = 1 << (log - log / 2);
		int yPow2 = 1 << (log / 2);
		assert xPow2 * yPow2 == pixels;
		mCharCols = xPow2 / (int) width;
		mCharRows = (int) Math.ceil((mLastChar - mFirstChar) / (float) mCharCols);
		yPow2 = mCharRows * (int) height;
		yPow2 = Util.roundToPowerOf2(yPow2);
		
		Bitmap bitmap = Bitmap.createBitmap(xPow2, yPow2, Bitmap.Config.ARGB_4444);
		Canvas canvas = new Canvas(bitmap);
		canvas.drawColor(0);
		
		float x = 0, y = -mFontMetrics.top;
		for (char ch = mFirstChar; ch < mLastChar; ++ch) {
			canvas.drawText(Character.toString(ch), x, y, paint);
			x += width;
			if (x + width > bitmap.getWidth()) {
				x = 0;
				y += height;
			}
		}

		mTexture = new GLESTexture(mName);
		boolean result = mTexture.init(GLESTexture.MinFilter.LINEAR, GLESTexture.MagFilter.LINEAR, GLESTexture.WrapMode.REPEAT, GLESTexture.WrapMode.REPEAT, bitmap);
		bitmap.recycle();
		
		return result;
	}
	
	public void done() {
		mTexture.done();
		mTexture = null;
	}
	
	@Override
	public void finalize() {
		done();
	}
	
	public RectF getCharUVRect(char ch) {
		if (ch < mFirstChar || ch >= mLastChar)
			ch = mFirstChar;
		int index = ch - mFirstChar;
		int row = index / mCharCols;
		int col = index % mCharCols;
		float cellWidth = mCharWidth / mTexture.getWidth();
		float cellHeight = (mFontMetrics.bottom - mFontMetrics.top) / mTexture.getHeight();
		RectF rect = new RectF();
		rect.left = col * cellWidth;
		rect.top = row * cellHeight;
		rect.right = rect.left + cellWidth;
		rect.bottom = rect.top + cellHeight;
		return rect;
	}
	
	public GLESUtil.VertexPosUV[] createVertices(String text, float x, float y, float viewportWidth, float viewportHeight) {
		GLESUtil.VertexPosUV[] vertices = new GLESUtil.VertexPosUV[text.length() * 4];
		
		return vertices;
	}
	
	public short[] createIndices(String text) {
		short[] indices = new short[text.length() * 6];
		
		return indices;
	}
}
