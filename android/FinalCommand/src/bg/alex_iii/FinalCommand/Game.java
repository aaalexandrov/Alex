package bg.alex_iii.FinalCommand;

import java.util.ArrayList;

import bg.alex_iii.GLES.Color;
import bg.alex_iii.GLES.Vec;

import android.os.SystemClock;
import android.view.MotionEvent;

public class Game {
	public MainRenderer mMainRenderer;
	
	public long mTime, mTimePrev, mClock;
	public Camera mCamera;
	public Terrain mTerrain;
	public ArrayList<GameObject> mObjects = new ArrayList<GameObject>();
	public RunnableList mModifications = new RunnableList();
	public boolean mIsUpdating = false;
	public Missile.Def mNukeDef, mSAMDef;
	
	TouchData[] mTouches = { new TouchData(), new TouchData() };
	
	class TouchData {
		int mPointerId = -1;
		float mX, mY, mInitialX, mInitialY;
		boolean mMoved;
		
		void init(int index, MotionEvent event) {
			mPointerId = event.getPointerId(index);
			mX = event.getX(index);
			mY = event.getY(index);
			mInitialX = mX;
			mInitialY = mY;
			mMoved = false;
		}
		
		boolean get(MotionEvent event) {
			int index = event.findPointerIndex(mPointerId);
			if (index >= 0) {
				boolean changed = mX != event.getX(index) || mY != event.getY(index);
				mX = event.getX(index);
				mY = event.getY(index);
				if (!mMoved && getMainView().lengthInInches(mX - mInitialX, mY - mInitialY) >= 0.2f)
					mMoved = true;
				return changed;
			} else
				invalidate();
			return false;
		}
		
		boolean isValid() {
			return mPointerId >= 0;
		}
		
		void invalidate() {
			mPointerId = -1;
		}
	}
	
	public Game(MainRenderer mainRenderer) {
		mMainRenderer = mainRenderer;
	}
	
	public boolean init() {
		mCamera = new Camera(this);
		mCamera.setPosition(10, 0, 10);
		
		mNukeDef = new Missile.Def(GameSettings.MISSILE_SPEED, Color.WHITE, 
									new Explosion.Def(GameSettings.EXPLOSION_DURATION, GameSettings.EXPLOSION_RADIUS, GameSettings.EXPLOSION_SPEED));
		mSAMDef = new Missile.Def(GameSettings.SAM_SPEED, Color.YELLOW, 
									new Explosion.Def(GameSettings.SAM_EXPLOSION_DURATION, GameSettings.SAM_EXPLOSION_RADIUS, GameSettings.SAM_EXPLOSION_SPEED));
		
		if (!initLevel())
			return false;
		
		mClock = SystemClock.uptimeMillis();
		mTime = mTimePrev = 0;
		return true;
	}
	
	public MainView getMainView() {
		return mMainRenderer.mActivity.mMainView;
	}
	
	public void addObject(final GameObject gameObject) {
		if (mIsUpdating) {
			mModifications.add(new Runnable() {
				public void run() {
					mObjects.add(gameObject);
				}
			});
		} else
			mObjects.add(gameObject);
	}
	
	public void removeObject(final GameObject gameObject) {
		if (mIsUpdating) {
			mModifications.add(new Runnable() {
				public void run() {
					mObjects.remove(gameObject);
				}
			});
		} else
			mObjects.remove(gameObject);
	}
	
	public float getDeltaTime() {
		return (mTime - mTimePrev) / 1000.0f;
	}
	
	public void update() {
		mModifications.run();
		mIsUpdating = true;
		
		long clockPrev = mClock;
		mClock = SystemClock.uptimeMillis();
		mTimePrev = mTime;
		mTime = Math.min(mTime + mClock - clockPrev, mTime + 1000);

		for (GameObject o: mObjects) 
			o.update();

		mIsUpdating = false;
		mModifications.run();
	}
	
	public boolean render() {
		boolean result = true;

		mCamera.update();
		
		result &= mTerrain.render();
		
		for (GameObject o: mObjects)
			result &= o.render();
		
		return result;
	}
	
	public GameObject createGameObject(GameObject.Def def, float x, float y, float z) {
		try {
			GameObject obj = def.gameObjectClass().newInstance();
			obj.init(this,  def);
			addObject(obj);
			obj.setPosition(x, y, z);
			return obj;
		} catch (Exception e) {
			return null;
		}
	}
	
	public GameObject createGameObject(GameObject.Def def, float x, float y) {
		return createGameObject(def, x, y, mTerrain.getHeight(x, y));
	}
	
	public boolean initLevel() {
		mTerrain = new Terrain(this, 24, 24, 1, 8);
		if (!mTerrain.init())
			return false;
		
		createGameObject(Target.DEF, -7, -7);
		createGameObject(Target.DEF, 7, -7);
		createGameObject(Target.DEF, -7, 7);
		createGameObject(Target.DEF, 7, 7);
		createGameObject(Target.DEF, 0, 0);
		
		createGameObject(MissileBase.DEF, -6, 0);
		createGameObject(MissileBase.DEF, 6, 0);
		createGameObject(MissileBase.DEF, 0, -6);
		createGameObject(MissileBase.DEF, 0, 6);
		
		return true;
	}
	
	public MissileBase getNearestBase(float[] position) {
		MissileBase result = null;
		float[] basePos = new float[3];
		float bestDist = Float.POSITIVE_INFINITY;
		for (GameObject o: mObjects) {
			if (o instanceof MissileBase) {
				o.getPosition(basePos);
				float dist = Vec.getLength(Vec.sub(position, basePos)); 
				if (dist < bestDist) {
					result = (MissileBase) o;
					bestDist = dist;
				}
			}
		}
		return result;
	}
	
	public void onTap(float x, float y) {
		float[] tapped, camPos = new float[3];
		mCamera.getPosition(camPos);
		tapped = mCamera.unProject(x, y);
		float[] direction = Vec.getNormalized(Vec.sub(tapped, camPos));
		float intersection = mTerrain.getRayIntersection(camPos, direction);
		if (Float.isNaN(intersection)) 
			return;
		float[] pos = Vec.add(camPos, Vec.mul(intersection, direction));
		Missile missile = (Missile) createGameObject(mNukeDef, ((float) Math.random() - 0.5f) * mTerrain.getSizeX(), ((float) Math.random() - 0.5f) * mTerrain.getSizeY(), GameSettings.MISSILE_START_ALTITUDE);
		missile.setTarget(pos[0], pos[1], pos[2]);
	}
	
	public void onAim(float x0, float y0, float x1, float y1) {
		if (Math.abs(x0 - x1) > 0.1f)
			return;
		if (y0 > y1) {
			float t = x0;
			x0 = x1;
			x1 = t;
			t = y0;
			y0 = y1;
			y1 = t;
		}
		float[] tappedBase, tappedUpper, camPos = new float[3];
		mCamera.getPosition(camPos);
		tappedBase = mCamera.unProject(x1, y1);
		float[] direction = Vec.getNormalized(Vec.sub(tappedBase, camPos));
		float intersection = mTerrain.getRayIntersection(camPos, direction);
		if (Float.isNaN(intersection)) 
			return;
		float[] pos = Vec.add(camPos, Vec.mul(intersection, direction));
		tappedUpper = mCamera.unProject(x0, y0);
		float[] upperDirection = Vec.getNormalized(Vec.sub(tappedUpper, camPos));
		float[] planeNormal = Vec.get(pos[0] - camPos[0], pos[1] - camPos[1], 0);
		float t = Vec.dot(Vec.sub(pos, camPos), planeNormal) / Vec.dot(upperDirection, planeNormal);
		float[] targetPos = Vec.add(camPos, Vec.mul(t, upperDirection));
		MissileBase base = getNearestBase(targetPos);
		if (base == null)
			return;
		float[] basePos = new float[3];
		base.getPosition(basePos);
		Missile missile = (Missile) createGameObject(mSAMDef, basePos[0], basePos[1], basePos[2] + GameSettings.BASE_HEIGHT);
		missile.setTarget(targetPos[0], targetPos[1], targetPos[2]);
	}
	
	public void endTouch(int touchIndex) {
		if (mTouches[touchIndex].isValid()) {
			int otherIndex = (touchIndex + 1) % 2;
			if (!mTouches[touchIndex].mMoved) {
				MainView mainView = getMainView();
				final float x = mTouches[touchIndex].mX / mainView.getWidth();
				final float y = mTouches[touchIndex].mY / mainView.getHeight();
				if (mTouches[otherIndex].isValid()) {
					final float x1 = mTouches[otherIndex].mX / mainView.getWidth();
					final float y1 = mTouches[otherIndex].mY / mainView.getHeight();
					mModifications.add(new Runnable() {
						public void run() {
							onAim(x, y, x1, y1);
						}
					});
				} else
					mModifications.add(new Runnable() {
						public void run() {
							onTap(x, y);
						}
					});
			} 
			mTouches[otherIndex].mMoved = true;
		}
		mTouches[touchIndex].invalidate();
	}
	
	public boolean onTouchEvent(MotionEvent event) {
		switch (event.getActionMasked()) {
			case MotionEvent.ACTION_DOWN: {
				mTouches[0].init(0, event);
				break;
			}
			case MotionEvent.ACTION_UP:
				for (int i = 0; i < 2; ++i)
					endTouch(i);
				break;
			case MotionEvent.ACTION_CANCEL:
				for (TouchData t: mTouches)
					t.invalidate();
				break;
			case MotionEvent.ACTION_POINTER_DOWN:
				if (!mTouches[1].isValid()) 
					mTouches[1].init(event.getActionIndex(), event);
				break;
			case MotionEvent.ACTION_POINTER_UP: {
				int pointerId = event.getPointerId(event.getActionIndex());
				if (mTouches[0].mPointerId == pointerId) {
					endTouch(0);
					TouchData t = mTouches[0];
					mTouches[0] = mTouches[1];
					mTouches[1] = t;
				} else
					if (mTouches[1].mPointerId == pointerId)
						endTouch(1);
				break;
			}
			case MotionEvent.ACTION_MOVE: {
				if (mTouches[0].isValid()) {
					float oldX, oldY, oldDist;
					float x, y, dist;

					if (mTouches[1].isValid()) {
						oldX = (mTouches[0].mX + mTouches[1].mX) * 0.5f;
						oldY = (mTouches[0].mY + mTouches[1].mY) * 0.5f;
						oldDist = Vec.getLength(Vec.get(mTouches[1].mX - mTouches[0].mX, mTouches[1].mY - mTouches[0].mY));
						boolean changed0 = mTouches[0].get(event);
						boolean changed1 = mTouches[1].get(event);
						if ((!changed0 || !mTouches[0].mMoved) && (!changed1 || !mTouches[1].mMoved))
							break;
						x = (mTouches[0].mX + mTouches[1].mX) * 0.5f;
						y = (mTouches[0].mY + mTouches[1].mY) * 0.5f;
						dist = Vec.getLength(Vec.get(mTouches[1].mX - mTouches[0].mX, mTouches[1].mY - mTouches[0].mY));
					} else {
						oldX = mTouches[0].mX;
						oldY = mTouches[0].mY;
						oldDist = dist = 0;
						boolean changed0 = mTouches[0].get(event);
						if (!changed0 || !mTouches[0].mMoved)
							break;
						x = mTouches[0].mX;
						y = mTouches[0].mY;
					}
					
					MainView mainView = getMainView();
					float scale = Math.min(mainView.getWidth(), mainView.getHeight());

					float[] pos = new float[3], lookAt = new float[3];
					mCamera.getPosition(pos);
					mCamera.getLookAt(lookAt);
					
					float[] polar = Vec.toPolar(Vec.sub(pos, lookAt));
					float r, theta, phi;
					r = polar[0];
					theta = polar[1];
					phi = polar[2];
					
					theta -= (y - oldY) / scale * (float) Math.PI;
					theta = Math.min(Math.max((float) Math.PI / 16, theta), (float) Math.PI * (1 - 1 / 16.0f));
					phi -= (x - oldX) / scale * (float) Math.PI;
					
					if (dist != oldDist) {
						r += (oldDist - dist) / scale * 30;
						r = Math.min(Math.max(5.0f, r), 50.0f);
					}
					
					pos = Vec.getPolar(r, theta, phi);
					pos = Vec.add(pos, lookAt);
					final float[] position = pos;
					mModifications.add(new Runnable() {
						public void run() {
							mCamera.setPosition(position[0], position[1], position[2]);
						}
					});
				}
				break;
			}
		}
		
		return true;
	}
}
