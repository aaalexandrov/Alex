package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

public class Target implements GameObject {
	Game mGame;
	GLESModel mModel;
	
	public Target(Game game) {
		mGame = game;
		
		GLESModel original = mGame.mMainRenderer.mPrism;
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
		return Shape.isPointInsideCylinder(point, Vec.get(curX, curY, curZ), GameSettings.TARGET_HEIGHT, GameSettings.TARGET_RADIUS);
	}
	
	public void setPosition(float x, float y, float z) {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.translateM(mModel.mTransform, 0, x, y, z);
	}
}
