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
		height = getCharHeight();
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
		
		Bitmap bitmap = Bitmap.createBitmap(xPow2, yPow2, Bitmap.Config.ALPHA_8);
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
		boolean result = mTexture.init(GLESTexture.MinFilter.LINEAR, GLESTexture.MagFilter.LINEAR, GLESTexture.WrapMode.CLAMP_TO_EDGE, GLESTexture.WrapMode.CLAMP_TO_EDGE, bitmap);
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
	
	float getCharHeight() {
		return (float) Math.ceil(mFontMetrics.bottom - mFontMetrics.top);
	}
	
	float getCellWidth() {
		return mCharWidth / mTexture.getWidth();
	}
	
	float getCellHeight() {
		return getCharHeight() / mTexture.getHeight();
	}
	
	public void getCharUVRect(char ch, float cellWidth, float cellHeight, RectF rect) {
		if (ch < mFirstChar || ch >= mLastChar)
			ch = mFirstChar;
		int index = ch - mFirstChar;
		int row = index / mCharCols;
		int col = index % mCharCols;
		rect.left = col * cellWidth;
		rect.top = row * cellHeight;
		rect.right = rect.left + cellWidth;
		rect.bottom = rect.top + cellHeight;
	}
	
	public GLESUtil.VertexPosUV[] createVertices(String text, float x, float y) {
		float cellWidth = getCellWidth();
		float cellHeight = getCellHeight();
		float quadWidth = mCharWidth;
		float quadHeight = getCharHeight();
		float curX = x;
		float curY = (y - (float) Math.ceil(mFontMetrics.top));
		float z = -1;
		
		GLESUtil.VertexPosUV[] vertices = new GLESUtil.VertexPosUV[text.length() * 4];
		int vert = 0;
		RectF uvRect = new RectF();
		for (int i = 0; i < text.length(); ++i) {
			getCharUVRect(text.charAt(i), cellWidth, cellHeight, uvRect);
			vertices[vert + 0] = new GLESUtil.VertexPosUV(curX, curY, z, uvRect.left, uvRect.top);
			vertices[vert + 1] = new GLESUtil.VertexPosUV(curX + quadWidth, curY, z, uvRect.right, uvRect.top);
			vertices[vert + 2] = new GLESUtil.VertexPosUV(curX, curY - quadHeight, z, uvRect.left, uvRect.bottom);
			vertices[vert + 3] = new GLESUtil.VertexPosUV(curX + quadWidth, curY - quadHeight, z, uvRect.right, uvRect.bottom);
			curX += quadWidth;
			vert += 4;
		}
		assert vert == vertices.length;
		
		return vertices;
	}
	
	public short[] createIndices(String text, int baseVertex) {
		short[] indices = new short[text.length() * 6];
		int vert = baseVertex;
		int ind = 0;
		for (int i = 0; i < text.length(); ++i) {
			indices[ind + 0] = (short) (vert + 0);
			indices[ind + 1] = (short) (vert + 2);
			indices[ind + 2] = (short) (vert + 1);

			indices[ind + 3] = (short) (vert + 1);
			indices[ind + 4] = (short) (vert + 2);
			indices[ind + 5] = (short) (vert + 3);
		
			vert += 4;
			ind += 6;
		}
		assert ind == indices.length;
		return indices;
	}
	
	public float[] setProjection(float viewportWidth, float viewportHeight, float[] transform) {
		transform[0] = 2 / viewportWidth;
		transform[1] = 0;
		transform[2] = 0;
		transform[3] = 0;
		
		transform[4] = 0;
		transform[5] = 2 / viewportHeight;
		transform[6] = 0;
		transform[7] = 0;
		
		transform[8] = 0;
		transform[9] = 0;
		transform[10] = 1;
		transform[11] = 0;
		
		transform[12] = -1;
		transform[13] = -1;
		transform[14] = 0;
		transform[15] = 1;
		return transform;
	}
}
