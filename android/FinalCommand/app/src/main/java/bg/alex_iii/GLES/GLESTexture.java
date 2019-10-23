package bg.alex_iii.GLES;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.util.Log;

public class GLESTexture {
	public static final String TAG = "GLESTexture";
	
	public enum MinFilter {
		NEAREST, LINEAR, NEAREST_MIPMAP_NEAREST, LINEAR_MIPMAP_NEAREST, NEAREST_MIPMAP_LINEAR, LINEAR_MIPMAP_LINEAR,
	}
	
	public enum MagFilter {
		NEAREST, LINEAR,
	}
	
	public enum WrapMode {
		CLAMP_TO_EDGE, MIRRORED_REPEAT, REPEAT,
	}
	
	protected String mName;
	protected int mTexture;
	protected int mWidth, mHeight;
	protected int mFormat;
	protected int mMinFilter, mMagFilter, mWrapS, mWrapT;
	
	public GLESTexture(String name) {
		mName = name;
		mTexture = 0;
	}
	
	public boolean init(MinFilter minFilter, MagFilter magFilter, WrapMode wrapS, WrapMode wrapT, Bitmap bitmap) {
		if (bitmap == null)
			return false;
		int[] tex = new int[1];
		GLES20.glGenTextures(1, tex, 0);
		mTexture = tex[0];
		mMinFilter = minFilter2GL(minFilter);
		mMagFilter = magFilter2GL(magFilter);
		mWrapS = wrapMode2GL(wrapS);
		mWrapT = wrapMode2GL(wrapT);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTexture);
		mWidth = bitmap.getWidth();
		mHeight = bitmap.getHeight();
		mFormat = GLUtils.getInternalFormat(bitmap);
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
		if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
			Log.e(TAG, "Error creating texture " + mName);
			return false;
		}
		if (minFilter != MinFilter.NEAREST && minFilter != MinFilter.LINEAR) {
			GLES20.glGenerateMipmap(GLES20.GL_TEXTURE_2D);
			if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
				Log.e(TAG, "Error generating mipmaps for texture " + mName);
				return false;
			}
		}
		return isValid();
	}
	
	public boolean init(MinFilter minFilter, MagFilter magFilter, WrapMode wrapS, WrapMode wrapT, Context context, int texResource) {
		Bitmap bitmap = BitmapFactory.decodeResource(context.getResources(), texResource);
		boolean res = init(minFilter, magFilter, wrapS, wrapT, bitmap);
		if (bitmap != null)
			bitmap.recycle();
		return res;
	}
	
	public void done() {
		if (mTexture != 0) {
			int[] tex = { mTexture };
			GLES20.glDeleteTextures(1, tex, 0 );
			mTexture = 0;
		}
	}
	
	@Override
	public void finalize() {
		done();
	}
	
	public boolean isValid() { return GLES20.glIsTexture(mTexture); }
	
	public String getName() { return mName; }
	public int getWidth() { return mWidth; }
	public int getHeight() { return mHeight; }
	public int getFormat() { return mFormat; }
	
	public boolean apply(int activeTexture) {
		GLES20.glActiveTexture(activeTexture);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTexture);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, mMinFilter);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, mMagFilter);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S, mWrapS);
		GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T, mWrapT);
		return GLES20.glGetError() == GLES20.GL_NO_ERROR; 
	}
	
	protected int minFilter2GL(MinFilter minFilter) {
		switch (minFilter) {
			case NEAREST:
				return GLES20.GL_NEAREST;
			case LINEAR:
				return GLES20.GL_LINEAR;
			case NEAREST_MIPMAP_NEAREST:
				return GLES20.GL_NEAREST_MIPMAP_NEAREST;
			case LINEAR_MIPMAP_NEAREST:
				return GLES20.GL_LINEAR_MIPMAP_NEAREST;
			case NEAREST_MIPMAP_LINEAR:
				return GLES20.GL_NEAREST_MIPMAP_LINEAR;
			case LINEAR_MIPMAP_LINEAR:
			default:
				return GLES20.GL_LINEAR_MIPMAP_LINEAR;
		}
	}

	protected int magFilter2GL(MagFilter magFilter) {
		switch (magFilter) {
			case NEAREST:
				return GLES20.GL_NEAREST;
			case LINEAR:
			default:
				return GLES20.GL_LINEAR;
		}
	}

	protected int wrapMode2GL(WrapMode wrapMode) {
		switch (wrapMode) {
			case CLAMP_TO_EDGE:
				return GLES20.GL_CLAMP_TO_EDGE;
			case MIRRORED_REPEAT:
				return GLES20.GL_MIRRORED_REPEAT;
			case REPEAT:
			default:
				return GLES20.GL_REPEAT;
		}
	}
	
}
