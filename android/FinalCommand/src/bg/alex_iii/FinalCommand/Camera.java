package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESCamera;
import bg.alex_iii.GLES.Vec;

public class Camera {
	private Game mGame;
	private float[] mLookAt, mPosition;
	
	public Camera(Game game) {
		mGame = game;
		mLookAt = Vec.getZero(3);
		mPosition = Vec.get(4.0f, 4.0f, 4.0f);
	}
	
	public synchronized float[] getPosition(float[] vec) {
		Vec.set(vec, mPosition[0], mPosition[1], mPosition[2]);
		return vec;
	}
	
	public synchronized void setPosition(float x, float y, float z) {
		Vec.set(mPosition, x, y, z);
	}
	
	public synchronized float[] getLookAt(float[] vec) {
		Vec.set(vec, mLookAt[0], mLookAt[1], mLookAt[2]);
		return vec;
	}
	
	public synchronized void setLookAt(float x, float y, float z) {
		Vec.set(mLookAt, x, y, z);
	}
	
	public synchronized void update() {
		GLESCamera camera = mGame.mMainRenderer.mRenderer.mCamera;
		camera.setTransform(mPosition, mLookAt, Vec.get(0, 0, 1));
	}
}
