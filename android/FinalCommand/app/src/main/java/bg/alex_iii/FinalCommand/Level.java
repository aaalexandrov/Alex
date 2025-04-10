package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.Color;
import bg.alex_iii.GLES.SoundPlayer;

public class Level {
	Game mGame;
	Missile.Def mNukeDef, mSAMDef;
	long mNextMissileTime, mLastEndCheckTime;
	
	Level(Game game) {
		mGame = game;
	
		SoundPlayer.Def explosionSound = game.mMainRenderer.mSoundPlayer.mDefs.get("explosion");
		SoundPlayer.Def explosionGroundSound = game.mMainRenderer.mSoundPlayer.mDefs.get("explosion_ground");
		mNukeDef = new Missile.Def(GameSettings.MISSILE_SPEED, Color.WHITE,
									new Explosion.Def(GameSettings.EXPLOSION_DURATION, 
														GameSettings.EXPLOSION_RADIUS, 
														GameSettings.EXPLOSION_SPEED, 
														true, 
														false, 
														explosionGroundSound,
														new Explosion.Def(GameSettings.EXPLOSION_DURATION,
																			GameSettings.SECONDARY_EXPLOSION_RADIUS,
																			GameSettings.SECONDARY_EXPLOSION_SPEED,
																			false,
																			true, 
																			null, 
																			null)));
		mSAMDef = new Missile.Def(GameSettings.SAM_SPEED, Color.YELLOW, 
									new Explosion.Def(GameSettings.SAM_EXPLOSION_DURATION, 
														GameSettings.SAM_EXPLOSION_RADIUS, 
														GameSettings.SAM_EXPLOSION_SPEED, 
														true, 
														false, 
														explosionSound, 
														null));
		
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
		mLastEndCheckTime = 0;
		
		return true;
	}

	long generateMissileTime() {
		return mGame.mTime + (long) (GameSettings.MISSILE_SPAWN_TIME * 1000 * (1 + (float) Math.random() * GameSettings.MISSILE_SPAWN_VARIATION * 2 - GameSettings.MISSILE_SPAWN_VARIATION));
	}
	
	boolean update() {
		int targetCount = 0, baseCount = 0;
		if (mNextMissileTime <= mGame.mTime || mGame.mTime >= mLastEndCheckTime + 1000)	{
			for (GameObject o: mGame.mObjects) {
				if (o instanceof Target) 
					++targetCount;
				else 
					if (o instanceof MissileBase) 
						++baseCount;
			}
			if (targetCount == 0) 
				return false;
			mLastEndCheckTime = mGame.mTime;
		}

		if (mNextMissileTime > mGame.mTime)
			return true;
		
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
