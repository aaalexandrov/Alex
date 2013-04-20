package bg.alex_iii.GLES;

import java.util.HashMap;

import android.content.Context;
import android.media.AudioManager;
import android.media.SoundPool;
import android.util.Log;
import android.util.SparseArray;

public class SoundPlayer implements SoundPool.OnLoadCompleteListener {
	public static final String TAG = "SoundPlayer";
	public static final int DEFAULT_PRIORITY = 100;

	public SoundPool mSoundPool;
	public Context mContext;
	public int mMaxStreams;
	public SparseArray<Def> mSoundID2Def = new SparseArray<Def>();
	public HashMap<String, Def> mDefs = new HashMap<String, Def>();

	public class Def {
		public String mName;
		public int mResourceID, mSoundID = 0;
		public int mPriority = DEFAULT_PRIORITY;
		public float mLeftVolume = 1, mRightVolume = 1, mRate = 1;
		public int mLoopCount = 0;
		public boolean mLoaded = false;

		public Def(String name, int resourceID, int priority) {
			mName = name;
			mResourceID = resourceID;
			mPriority = priority;
			init();
		}
		
		@Override
		public void finalize() {
			done();
		}
		
		public void init() {
			mSoundID = mSoundPool.load(mContext, mResourceID, mPriority);
			assert !mDefs.containsKey(mName) && mSoundID2Def.get(mResourceID) == null;
			mSoundID2Def.put(mSoundID, this);
			mDefs.put(mName, this);
		}
		
		public void done() {
			mSoundID2Def.remove(mResourceID);
			mDefs.remove(mName);
			mSoundPool.unload(mSoundID);
			mLoaded = false;
			mSoundID = 0;
		}
		
		public int play() {
			if (!mLoaded) {
				Log.e(TAG, String.format("Def.play(): Trying to play sound that is not loaded, def name %s", mName));
				return 0;
			}
			int streamID = mSoundPool.play(mSoundID, mLeftVolume, mRightVolume, mPriority, mLoopCount, mRate);
			return streamID;
		}
	}

	public SoundPlayer(Context context, int maxStreams) {
		mContext = context;
		mMaxStreams = maxStreams;
		init();
	}

	@Override
	public void finalize() {
		done();
	}
	
	public void init() {
		mSoundPool = new SoundPool(mMaxStreams, AudioManager.STREAM_MUSIC, 0);
		mSoundPool.setOnLoadCompleteListener(this);
	}

	public void done() {
		mSoundID2Def.clear();
		mDefs.clear();
		mSoundPool.release();
		mSoundPool = null;
	}

	public Def loadSound(String name, int resourceID, int priority) {
		Def def = new Def(name, resourceID, priority);
		return def;
	}
	
	public void onLoadComplete(SoundPool soundPool, int sampleID, int status) {
		Def def = mSoundID2Def.get(sampleID);
		if (def == null) {
			Log.e(TAG, String.format("onLoadComplete(): could not find def for sampleID %d which finished loading with status code %d", sampleID, status));
			return;
		}
		if (status != 0) {
			Log.e(TAG, String.format("onLoadComplete(): error loading sampleID %d - status code %d", sampleID, status));
			return;
		}
		def.mLoaded = true;
	}
}
