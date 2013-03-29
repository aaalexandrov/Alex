package bg.alex_iii.FinalCommand;

import java.util.ArrayList;

import bg.alex_iii.GLES.Vec;

import android.os.SystemClock;
import android.view.MotionEvent;

public class Game {
	public MainRenderer mMainRenderer;
	
	public long mTime, mTimePrev;
	public Camera mCamera;
	public Terrain mTerrain;
	public ArrayList<GameObject> mObjects = new ArrayList<GameObject>();
	public ArrayList<Runnable> mModifications = new ArrayList<Runnable>();
	public boolean mIsUpdating = false;
	
	TouchData[] mTouches = { new TouchData(), new TouchData() };
	
	static class TouchData {
		int mPointerId = -1;
		float mX, mY;
		
		void init(int index, MotionEvent event) {
			mPointerId = event.getPointerId(index);
			mX = event.getX(index);
			mY = event.getY(index);
		}
		
		void get(MotionEvent event) {
			int index = event.findPointerIndex(mPointerId);
			if (index >= 0) {
				mX = event.getX(index);
				mY = event.getY(index);
			} else
				invalidate();
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
		
		mTime = SystemClock.uptimeMillis();
		return true;
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
	
	public void runModifications() {
		for (Runnable r: mModifications) 
			r.run();
		mModifications.clear();
	}
	
	public void update() {
		mIsUpdating = true;
		
		mTimePrev = mTime;
		mTime = SystemClock.uptimeMillis();

		for (GameObject o: mObjects) 
			o.update();

		mIsUpdating = false;
		runModifications();
	}
	
	public boolean render() {
		boolean result = true;

		mCamera.update();
		
		result &= mTerrain.render();
		
		for (GameObject o: mObjects)
			result &= o.render();
		
		return result;
	}
	
	public void createTarget(float x, float y) {
		Target target = new Target(this);
		target.setPosition(x, y, mTerrain.getHeight(x, y));
		addObject(target);
	}
	
	public boolean initLevel() {
		mTerrain = new Terrain(this, 16, 16, 1);
		if (!mTerrain.init())
			return false;
		
		createTarget(-7, -7);
		createTarget(7, -7);
		createTarget(-7, 7);
		createTarget(7, 7);
		createTarget(0, 0);
		
		return true;
	}
	
	public boolean onTouchEvent(MainView mainView, MotionEvent event) {
		switch (event.getActionMasked()) {
			case MotionEvent.ACTION_DOWN:
				mTouches[0].init(0, event);
				break;
			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_CANCEL:
				for (TouchData t: mTouches)
					t.invalidate();
				break;
			case MotionEvent.ACTION_POINTER_DOWN:
				if (!mTouches[1].isValid()) 
					mTouches[1].init(event.getActionIndex(), event);
				break;
			case MotionEvent.ACTION_POINTER_UP:
				if (mTouches[1].mPointerId == event.getPointerId(event.getActionIndex()))
					mTouches[1].invalidate();
				break;
			case MotionEvent.ACTION_MOVE: {
				if (mTouches[0].isValid()) {
					float oldX = mTouches[0].mX;
					float oldY = mTouches[0].mY;

					mTouches[0].get(event);
					
					float scale = Math.min(mainView.getWidth(), mainView.getHeight());

					float[] pos = new float[3], lookAt = new float[3];
					mCamera.getPosition(pos);
					mCamera.getLookAt(lookAt);
					
					float[] polar = Vec.toPolar(Vec.sub(pos, lookAt));
					float r, theta, phi;
					r = polar[0];
					theta = polar[1];
					phi = polar[2];
					
					theta -= (mTouches[0].mY - oldY) / scale * (float) Math.PI;
					theta = Math.min(Math.max((float) Math.PI / 16, theta), (float) Math.PI * (1 - 1 / 16.0f));
					phi -= (mTouches[0].mX - oldX) / scale * (float) Math.PI;
					
					if (mTouches[1].isValid()) {
						float oldScaleX = mTouches[1].mX;
						float oldScaleY = mTouches[1].mY;
						mTouches[1].get(event);
						
						float oldDist = Vec.getLength(Vec.get(oldScaleX - oldX, oldScaleY - oldY));
						float dist = Vec.getLength(Vec.get(mTouches[1].mX - mTouches[0].mX, mTouches[1].mY - mTouches[0].mY));
						
						r += (dist - oldDist) / scale * 10;
						r = Math.min(Math.max(5.0f, r), 50.0f);
					}
					
					pos = Vec.getPolar(r, theta, phi);
					pos = Vec.add(pos, lookAt);
					mCamera.setPosition(pos[0], pos[1], pos[2]);
				}
				break;
			}
				
			
		}
		
		return true;
	}
}
