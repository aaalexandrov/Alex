package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.SoundPlayer;
import bg.alex_iii.GLES.Vec;

public class Explosion implements GameObject {
	Game mGame;
	Def mDef;
	GLESModel mModel;
	float mRadius;
	float[] mPosition;
	long mCreationTime;
	
	public static class Def implements GameObject.Def {
		float mDuration;
		float mRadius;
		float mSpeed;
		SoundPlayer.Def mSound;
		
		public Def(float duration, float radius, float speed, SoundPlayer.Def sound) {
			mDuration = duration;
			mRadius = radius;
			mSpeed = speed;
			mSound = sound;
		}
		
		public Class<? extends GameObject> gameObjectClass() {
			return Explosion.class;
		}
	}
	
	public boolean init(Game game, GameObject.Def def) {
		mGame = game;
		mDef = (Def) def;
		
		GLESModel original = mGame.mMainRenderer.mSphere;
		float[] transform = new float[16];
		Matrix.setIdentityM(new float[16], 0);
		mModel = new GLESModel(original.mGeometry, original.mMaterial, transform);
		
		mRadius = 0;
		mPosition = Vec.getZero(3);
		mCreationTime = mGame.mTime;
		
		if (mDef.mSound != null)
			mDef.mSound.play();
		
		return true;
	}
	
	public boolean render() {
		return mModel.render();
	}
	
	public void update() {
		if ((mGame.mTime - mCreationTime) / 1000.0f > mDef.mDuration) {
			mGame.removeObject(this);
			return;
		}
		
		mRadius += mDef.mSpeed * mGame.getDeltaTime();
		mRadius = Math.min(mRadius, mDef.mRadius);
		
		float[] objPos = new float[3];
		for (GameObject o: mGame.mObjects) {
			if (o instanceof Explosion)
				continue;
			o.getPosition(objPos);
			if (Vec.getLengthSquare(Vec.sub(mPosition, objPos)) <= mRadius * mRadius) {
				mGame.removeObject(o);
			}
		}
		
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
	
	public float[] getPosition(float[] position) {
		Vec.copy(position, mPosition);
		return position;
	}
	
	public void setPosition(float x, float y, float z) {
		Vec.set(mPosition, x, y, z);
		updateTransform();
	}

}
