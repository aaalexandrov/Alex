package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

public class Explosion implements GameObject {
	Game mGame;
	GLESModel mModel;
	float mRadius;
	long mCreationTime;
	
	Explosion(Game game) {
		mGame = game;
		
		GLESModel original = mGame.mMainRenderer.mSphere;
		float[] transform = new float[16];
		Matrix.setIdentityM(new float[16], 0);
		mModel = new GLESModel(original.mGeometry, original.mMaterial, transform);
		
		mRadius = 0;
		mCreationTime = mGame.mTime;
	}
	
	public Game getGame() {
		return mGame;
	}
	
	public boolean render() {
		return mModel.render();
	}
	
	public void update() {
		if ((mGame.mTime - mCreationTime) / 1000.0f > GameSettings.EXPLOSION_DURATION) {
			mGame.removeObject(this);
			return;
		}
		
		float curX, curY, curZ;
		curX = mModel.mTransform[12];
		curY = mModel.mTransform[13];
		curZ = mModel.mTransform[14];

		mRadius += GameSettings.EXPLOSION_SPEED * mGame.getDeltaTime();
		mRadius = Math.min(mRadius, GameSettings.EXPLOSION_RADIUS);
		
		setPosition(curX, curY, curZ);
	}
	
	public boolean isPointInside(float[] point) {
		float curX, curY, curZ;
		curX = mModel.mTransform[12];
		curY = mModel.mTransform[13];
		curZ = mModel.mTransform[14];
		return Shape.isPointInsideSphere(point, Vec.get(curX, curY, curZ), mRadius);
	}
	
	public void setPosition(float x, float y, float z) {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.scaleM(mModel.mTransform, 0, mRadius, mRadius, mRadius);
		Matrix.translateM(mModel.mTransform, 0, x, y, z);
	}
}
