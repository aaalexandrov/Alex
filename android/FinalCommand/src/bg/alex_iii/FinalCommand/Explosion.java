package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESMaterial;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESSorter;
import bg.alex_iii.GLES.GLESState;
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
		boolean mDoDamage;
		boolean mAnimateAlpha;
		SoundPlayer.Def mSound;
		Def mSecondary;
		
		public Def(float duration, float radius, float speed, boolean doDamage, boolean animateAlpha, SoundPlayer.Def sound, Def secondary) {
			mDuration = duration;
			mRadius = radius;
			mSpeed = speed;
			mDoDamage = doDamage;
			mAnimateAlpha = animateAlpha;
			mSound = sound;
			mSecondary = secondary;
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
		GLESMaterial material;
		if (mDef.mAnimateAlpha) {
			material = new GLESMaterial(original.mMaterial.mName + hashCode(), original.mMaterial.mShader, new GLESState());
			material.copyFrom(original.mMaterial);
			material.mState.setBlendMode(GLESState.BlendMode.BLEND);
		} else
			material = original.mMaterial;
		mModel = new GLESModel(original.mGeometry, material, transform);
		
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
	
	public boolean addToSorter(GLESSorter sorter) {
		return sorter.add(mModel);
	}
	
	public void update() {
		if (mGame.mTime - mCreationTime > mDef.mDuration * 1000) {
			mGame.removeObject(this);
			return;
		}
		
		mRadius += mDef.mSpeed * mGame.getDeltaTime();
		mRadius = Math.min(mRadius, mDef.mRadius);
		
		if (mDef.mDoDamage) {
			float[] objPos = new float[3];
			for (GameObject o: mGame.mObjects) {
				if (o instanceof Explosion)
					continue;
				o.getPosition(objPos);
				if (Vec.getLengthSquare(Vec.sub(mPosition, objPos)) <= mRadius * mRadius) {
					mGame.removeObject(o);
				}
			}
		}
		
		updateModel();
	}
	
	public boolean isPointInside(float[] point) {
		return Shape.isPointInsideSphere(point, mPosition, mRadius);
	}
	
	public void updateModel() {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.translateM(mModel.mTransform, 0, mPosition[0], mPosition[1], mPosition[2]);
		Matrix.scaleM(mModel.mTransform, 0, mRadius, mRadius, mRadius);
		
		if (mDef.mAnimateAlpha) {
			float alpha = 1 - (mGame.mTime - mCreationTime) / (mDef.mDuration * 1000);
			int colorIndex = mModel.mMaterial.getUniformIndex("uColor");
			mModel.mMaterial.mUniforms[colorIndex].mValue[3] = alpha;
		}
	}
	
	public float[] getPosition(float[] position) {
		Vec.copy(position, mPosition);
		return position;
	}
	
	public void setPosition(float x, float y, float z) {
		Vec.set(mPosition, x, y, z);
		if (mDef.mSecondary != null) 
			mGame.createGameObject(mDef.mSecondary, mPosition[0], mPosition[1], mPosition[2]);
		updateModel();
	}
}

