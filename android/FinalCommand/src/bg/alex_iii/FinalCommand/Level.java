package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.Color;

public class Level {
	Game mGame;
	Missile.Def mNukeDef, mSAMDef;
	long mNextMissileTime;
	
	Level(Game game) {
		mGame = game;
	
		mNukeDef = new Missile.Def(GameSettings.MISSILE_SPEED, Color.WHITE,
									new Explosion.Def(GameSettings.EXPLOSION_DURATION, GameSettings.EXPLOSION_RADIUS, GameSettings.EXPLOSION_SPEED));
		mSAMDef = new Missile.Def(GameSettings.SAM_SPEED, Color.YELLOW, 
									new Explosion.Def(GameSettings.SAM_EXPLOSION_DURATION, GameSettings.SAM_EXPLOSION_RADIUS, GameSettings.SAM_EXPLOSION_SPEED));
		
		initLevel();
	}
	
	boolean initLevel() {
		mGame.mTerrain = new Terrain(mGame, 24, 24, 1, 8);
		if (!mGame.mTerrain.init())
			return false;

		mGame.mObjects.clear();
		
		mGame.createGameObject(Target.DEF, -7, -7);
		mGame.createGameObject(Target.DEF, 7, -7);
		mGame.createGameObject(Target.DEF, -7, 7);
		mGame.createGameObject(Target.DEF, 7, 7);
		mGame.createGameObject(Target.DEF, 0, 0);
		
		mGame.createGameObject(MissileBase.DEF, -6, 0);
		mGame.createGameObject(MissileBase.DEF, 6, 0);
		mGame.createGameObject(MissileBase.DEF, 0, -6);
		mGame.createGameObject(MissileBase.DEF, 0, 6);
		
		mNextMissileTime = generateMissileTime();
		
		return true;
	}

	long generateMissileTime() {
		return mGame.mTime + (long) (GameSettings.MISSILE_SPAWN_TIME * 1000 * (0.7f + (float) Math.random() * 0.6f));
	}
	
	boolean update() {
		if (mNextMissileTime > mGame.mTime)
			return true;
		
		int targetCount = 0, baseCount = 0;
		for (GameObject o: mGame.mObjects) {
			if (o instanceof Target) 
				++targetCount;
			else 
				if (o instanceof MissileBase) 
					++baseCount;
		}
		if (targetCount == 0) 
			return false;

		int targetIndex = (int) (Math.random() * (targetCount + baseCount));
		for (GameObject o: mGame.mObjects) {
			if ((o instanceof Target || o instanceof MissileBase) && --targetIndex == 0) {
				Missile missile = (Missile) mGame.createGameObject(mNukeDef, 
																	((float) Math.random() - 0.5f) * mGame.mTerrain.getSizeX(), 
																	((float) Math.random() - 0.5f) * mGame.mTerrain.getSizeY(), 
																	GameSettings.MISSILE_START_ALTITUDE);
				float[] pos = new float[3];
				o.getPosition(pos);
				missile.setTarget(pos[0], pos[1], pos[2]);
				break;
			}
		}

		mNextMissileTime = generateMissileTime();
		
		return true;
	}
}
