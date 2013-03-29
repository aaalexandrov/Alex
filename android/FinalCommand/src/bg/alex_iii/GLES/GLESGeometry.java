package bg.alex_iii.GLES;

import android.opengl.GLES20;

public class GLESGeometry {
	public static final String TAG = "GLESGeometry";

	public enum PrimitiveType {
        POINTS, LINE_STRIP, LINE_LOOP, LINES, TRIANGLE_STRIP, TRIANGLE_FAN, TRIANGLES,
	}
	
	public GLESBuffer mVertices;
	public GLESBuffer mIndices;
	public int mPrimitiveType;
	
	public GLESGeometry(GLESBuffer vertices, GLESBuffer indices, PrimitiveType primitiveType) {
		mVertices = vertices;
		mIndices = indices;
		assert(mIndices == null || mIndices.isIndexBuffer());
		mPrimitiveType = primitiveType2GL(primitiveType);
	}
	
	public boolean apply(GLESShader shader) {
		if (!mVertices.apply(shader.mAttribs))
			return false;
		if (mIndices != null) {
			if (!mIndices.apply(null))
				return false;
		}
		return true;
	}
	
	public boolean render() {
		if (mIndices != null)
			GLES20.glDrawElements(mPrimitiveType, mIndices.getCount(), mIndices.mFormat.getParam(0).mType, 0);
		else
			GLES20.glDrawArrays(mPrimitiveType, 0, mVertices.getCount());
		return GLES20.glGetError() == GLES20.GL_NO_ERROR;
	}
	
	protected int primitiveType2GL(PrimitiveType primitiveType) {
		switch (primitiveType) {
			case POINTS:
				return GLES20.GL_POINTS;
			case LINE_STRIP:
				return GLES20.GL_LINE_STRIP;
			case LINE_LOOP:
				return GLES20.GL_LINE_LOOP;
			case LINES:
				return GLES20.GL_LINES;
			case TRIANGLE_STRIP:
				return GLES20.GL_TRIANGLE_STRIP;
			case TRIANGLE_FAN:
				return GLES20.GL_TRIANGLE_FAN;
			case TRIANGLES:
			default:
				return GLES20.GL_TRIANGLES;
		}
	}
}
