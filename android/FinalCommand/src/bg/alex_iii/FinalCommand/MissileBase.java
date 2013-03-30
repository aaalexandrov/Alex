package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

public class MissileBase implements GameObject {
	public Game mGame;
	public GLESModel mModel;
	
	public MissileBase(Game game) {
		mGame = game;

		GLESModel original = mGame.mMainRenderer.mCone;
		float[] transform = new float[16];
		Matrix.setIdentityM(new float[16], 0);
		mModel = new GLESModel(original.mGeometry, original.mMaterial, transform);
	}

	public Game getGame() {
		return mGame;
	}

	public boolean render() {
		return mModel.render();
	}

	public void update() {
	}

	public boolean isPointInside(float[] point) {
		float curX, curY, curZ;
		curX = mModel.mTransform[12];
		curY = mModel.mTransform[13];
		curZ = mModel.mTransform[14];
		return Shape.isPointInsideCone(point, Vec.get(curX, curY, curZ), GameSettings.BASE_HEIGHT, GameSettings.BASE_RADIUS);
	}
	
	public void setPosition(float x, float y, float z) {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.translateM(mModel.mTransform, 0, x, y, z);
	}
}
