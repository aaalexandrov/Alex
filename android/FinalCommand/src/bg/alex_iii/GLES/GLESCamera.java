package bg.alex_iii.GLES;

import android.opengl.Matrix;

public class GLESCamera {
	public static final String TAG = "GLESCamera";
	
	public float[] mTransform = new float[16];
	public float[] mView = new float[16];
	public float[] mProj = new float[16];
	public boolean mIsOrtho;
	public float mNear, mFar, mLeft, mRight, mTop, mBottom;
	
	public GLESCamera() {
		Matrix.setIdentityM(mTransform, 0);
		Matrix.setIdentityM(mView, 0);
		setProjection(-1, 1, -1, 1, 0.1f, 1);
	}
	
	public float[] getViewProj() {
		float[] viewProj = new float[16];
		Matrix.multiplyMM(viewProj, 0, mProj, 0, mView, 0);
		return viewProj;
	}
	
	public void setProjection(float left, float right, float bottom, float top, float near, float far) {
		mIsOrtho = false;
		mNear = near;
		mFar = far;
		mLeft = left;
		mRight = right;
		mTop = top;
		mBottom = bottom;
		Matrix.frustumM(mProj, 0, left, right, bottom, top, near, far);
	}
	
	public void setOrtho(float left, float right, float bottom, float top, float near, float far) {
		mIsOrtho = true;
		mNear = near;
		mFar = far;
		mLeft = left;
		mRight = right;
		mTop = top;
		mBottom = bottom;
		Matrix.orthoM(mProj, 0, left, right, bottom, top, near, far);
	}
	
	public void setTransform(float[] eyePosition, float[] lookAtPoint, float[] upDirection) {
		Matrix.setLookAtM(mView, 0, eyePosition[0], eyePosition[1], eyePosition[2], 
				lookAtPoint[0], lookAtPoint[1], lookAtPoint[2], 
				upDirection[0], upDirection[1], upDirection[2]);
		Matrix.invertM(mTransform, 0, mView, 0);
	}
	
	public void setTransform(float[] transform) {
		assert(transform.length == mTransform.length);
		mTransform = transform;
		Matrix.invertM(mView, 0, mTransform, 0);
	}
	
	public float[] unProject(float[] position) {
		float[] viewProj = getViewProj();
		float[] invViewProj = new float[16];
		Matrix.invertM(invViewProj, 0, viewProj, 0);
		float[] result = new float[4];
		Matrix.multiplyMV(result, 0, invViewProj, 0, position, 0);
		return result;
	}
	
	public float[] unProject(float x, float y) {
		float[] pos = Vec.get(x * (mRight - mLeft) + mLeft, y * (mBottom - mTop) - mTop, mNear, 1);
		return unProject(pos);
	}
}
