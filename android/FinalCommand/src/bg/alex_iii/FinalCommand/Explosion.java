package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

public class Explosion implements GameObject {
	Game mGame;
	GLESModel mModel;
	float mRadius;
	float[] mPosition;
	long mCreationTime;
	
	public Explosion(Game game) {
		mGame = game;
		
		GLESModel original = mGame.mMainRenderer.mSphere;
		float[] transform = new float[16];
		Matrix.setIdentityM(new float[16], 0);
		mModel = new GLESModel(original.mGeometry, original.mMaterial, transform);
		
		mRadius = 0;
		mPosition = Vec.getZero(3);
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
		
		mRadius += GameSettings.EXPLOSION_SPEED * mGame.getDeltaTime();
		mRadius = Math.min(mRadius, GameSettings.EXPLOSION_RADIUS);
		
		updateTransform();
	}
	
	public boolean isPointInside(float[] point) {
		return Shape.isPointInsideSphere(point, mPosition, mRadius);
	}
	
	public void updateTransform() {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.translateM(mModel.mTransform, 0, mPosition[0], mPosition[1], mPosition[2]);
		Matrix.scaleM(mModel.mTransform, 0, mRadius, mRadius, mRadius);
	}
	
	public void setPosition(float x, float y, float z) {
		Vec.set(mPosition, x, y, z);
		updateTransform();
	}
}
