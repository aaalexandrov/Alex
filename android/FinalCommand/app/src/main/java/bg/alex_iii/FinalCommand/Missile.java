package bg.alex_iii.FinalCommand;

import java.util.ArrayList;

import bg.alex_iii.GLES.Color;
import bg.alex_iii.GLES.GLESSorter;
import bg.alex_iii.GLES.Vec;

public class Missile implements GameObject {
	static final float SHADOW_DISPLACEMENT = 0.1f;
	
	Game mGame;
	Def mDef;
	float[] mOrigin, mTarget, mPosition;
	
	public static class Def implements GameObject.Def {
		float mSpeed;
		byte[] mTrailColor;
		Explosion.Def mExplosionDef;

		public Def(float speed, byte[] trailColor, Explosion.Def explosionDef) {
			mSpeed = speed;
			mTrailColor = trailColor;
			mExplosionDef = explosionDef;
		}
		
		public Class<? extends GameObject> gameObjectClass() {
			return Missile.class;
		}
	}
	
	public boolean init(Game game, GameObject.Def def) {
		mGame = game;
		mDef = (Def) def;
		return true;
	}
	
	public boolean render() {
		return true;
	}
	
	public boolean addToSorter(GLESSorter sorter) {
		return true;
	}
	
	public void update() {
		float[] delta = Vec.sub(mTarget, mPosition);
		float dist = Vec.getLength(delta);
		float travelDist = mDef.mSpeed * mGame.getDeltaTime();
		if (dist <= travelDist) {
			mGame.removeObject(this);
			mGame.createGameObject(mDef.mExplosionDef, mTarget[0], mTarget[1], mTarget[2]);
			return;
		}
		mPosition = Vec.add(mPosition, Vec.mul(delta, travelDist / dist));
		
		mGame.mMainRenderer.mLineHolder.setColor(mDef.mTrailColor);
		mGame.mMainRenderer.mLineHolder.addPoint(mOrigin[0], mOrigin[1], mOrigin[2]);
		mGame.mMainRenderer.mLineHolder.addPoint(mPosition[0], mPosition[1], mPosition[2]);
		
		ArrayList<Float> factors = new ArrayList<Float>();
		mGame.mTerrain.breakLine(mOrigin, mPosition, factors);
		if (factors.size() > 1) {
			mGame.mMainRenderer.mLineHolder.setColor(Color.BLACK);

			float x = (mPosition[0] - mOrigin[0]) * factors.get(0) + mOrigin[0]; 
			float y = (mPosition[1] - mOrigin[1]) * factors.get(0) + mOrigin[1];
			float z = mGame.mTerrain.getHeight(x, y) + SHADOW_DISPLACEMENT;
			for (int i = 1; i < factors.size(); ++i) {
				mGame.mMainRenderer.mLineHolder.addPoint(x, y, z);
				x = (mPosition[0] - mOrigin[0]) * factors.get(i) + mOrigin[0]; 
				y = (mPosition[1] - mOrigin[1]) * factors.get(i) + mOrigin[1];
				z = mGame.mTerrain.getHeight(x, y) + SHADOW_DISPLACEMENT;
				mGame.mMainRenderer.mLineHolder.addPoint(x, y, z);
			}
		}
	}

	public void setTarget(float x, float y, float z) {
		mTarget = Vec.get(x, y, z); 
	}
	
	public float[] getPosition(float[] position) {
		Vec.copy(position, mPosition);
		return position;
	}
	
	public void setPosition(float x, float y, float z) {
		if (mOrigin == null)
			mOrigin = Vec.get(x,  y, z);
		mPosition = Vec.get(x, y, z);
	}
}
