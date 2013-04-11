package bg.alex_iii.FinalCommand;

import java.lang.reflect.Constructor;
import java.util.ArrayList;

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
	
	public <T extends GameObject> T createGameObject(Class<T> objClass, float x, float y, float z) {
		try {
			Constructor<T> constructor = objClass.getConstructor(Game.class);
			T obj = constructor.newInstance(this);
			addObject(obj);
			obj.setPosition(x, y, z);
			return obj;
		} catch (Exception e) {
			return null;
		}
	}
	
	public <T extends GameObject> T createGameObject(Class<T> objClass, float x, float y) {
		return createGameObject(objClass, x, y, mTerrain.getHeight(x, y));
	}
	
	public boolean initLevel() {
		mTerrain = new Terrain(this, 24, 24, 1, 8);
		if (!mTerrain.init())
			return false;
		
		createGameObject(Target.class, -7, -7);
		createGameObject(Target.class, 7, -7);
		createGameObject(Target.class, -7, 7);
		createGameObject(Target.class, 7, 7);
		createGameObject(Target.class, 0, 0);
		
		createGameObject(MissileBase.class, -4, 0);
		createGameObject(MissileBase.class, 4, 0);
		createGameObject(MissileBase.class, 0, -4);
		createGameObject(MissileBase.class, 0, 4);
		
		return true;
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
		createGameObject(Explosion.class, pos[0], pos[1], pos[2]);
		pos = Vec.add(camPos, Vec.mul(5, direction));
	}
	
	public void endTouch(int touchIndex) {
		if (mTouches[touchIndex].isValid()) {
			if (!mTouches[touchIndex].mMoved) {
				MainView mainView = getMainView();
				final float x = mTouches[touchIndex].mX / mainView.getWidth();
				final float y = mTouches[touchIndex].mY / mainView.getHeight(); 
				mModifications.add(new Runnable() {
					public void run() {
						onTap(x, y);
					}
				});
			} else
				mTouches[(touchIndex + 1) % 2].mMoved = true;
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
