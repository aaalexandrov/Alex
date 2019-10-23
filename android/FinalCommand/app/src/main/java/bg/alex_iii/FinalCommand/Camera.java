package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESCamera;
import bg.alex_iii.GLES.Vec;

public class Camera {
	private Game mGame;
	private float[] mLookAt, mPosition;
	
	public Camera(Game game) {
		mGame = game;
		mLookAt = Vec.get(0, 0, GameSettings.MISSILE_START_ALTITUDE / 3);
		mPosition = Vec.get(4.0f, 4.0f, 4.0f);
	}
	
	public float[] getPosition(float[] vec) {
		Vec.set(vec, mPosition[0], mPosition[1], mPosition[2]);
		return vec;
	}
	
	public void setPosition(float x, float y, float z) {
		Vec.set(mPosition, x, y, z);
	}
	
	public float[] getLookAt(float[] vec) {
		Vec.set(vec, mLookAt[0], mLookAt[1], mLookAt[2]);
		return vec;
	}
	
	public void setLookAt(float x, float y, float z) {
		Vec.set(mLookAt, x, y, z);
	}

	public float[] unProject(float x, float y) {
		return mGame.mMainRenderer.mRenderer.mCamera.unProject(x, y);
	}
	
	public void update() {
		GLESCamera camera = mGame.mMainRenderer.mRenderer.mCamera;
		camera.setTransform(mPosition, mLookAt, Vec.get(0, 0, 1));
	}
	
}
