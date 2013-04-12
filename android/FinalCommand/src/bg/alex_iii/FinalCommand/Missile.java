package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.Vec;

public class Missile implements GameObject {
	Game mGame;
	GLESModel mModel;
	float[] mOrigin, mTarget, mPosition;
	
	public Missile(Game game) {
		mGame = game;
	}
	
	public Game getGame() {
		return mGame;
	}
	
	public boolean render() {
		return true;
	}
	
	public void update() {
		float[] delta = Vec.sub(mTarget, mPosition);
		float dist = Vec.getLength(delta);
		float travelDist = GameSettings.MISSILE_SPEED * mGame.getDeltaTime();
		if (dist <= travelDist) {
			mGame.removeObject(this);
			mGame.createGameObject(Explosion.class, mTarget[0], mTarget[1], mTarget[2]);
			return;
		}
		mPosition = Vec.add(mPosition, Vec.mul(delta, travelDist / dist));
		
		mGame.mMainRenderer.mLineHolder.addPoint(mOrigin[0], mOrigin[1], mOrigin[2]);
		mGame.mMainRenderer.mLineHolder.addPoint(mPosition[0], mPosition[1], mPosition[2]);
	}

	public void setTarget(float x, float y, float z) {
		mTarget = Vec.get(x, y, z); 
	}
	
	public void setPosition(float x, float y, float z) {
		if (mOrigin == null)
			mOrigin = Vec.get(x,  y, z);
		mPosition = Vec.get(x, y, z);
	}
}
