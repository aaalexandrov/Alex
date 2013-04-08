package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESRenderer;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.DisplayMetrics;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.content.Context;

public class MainView extends GLSurfaceView {

	public GLESRenderer mGLESRenderer;
	public MainRenderer mMainRenderer;
	public float mIPDx, mIPDy;
	
	public MainView(Context context, AttributeSet attrs) {
		super(context, attrs);
		mMainRenderer = new MainRenderer();
		mGLESRenderer = new GLESRenderer(context, this, mMainRenderer);
		
		DisplayMetrics metrics = new DisplayMetrics();
		WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
		windowManager.getDefaultDisplay().getMetrics(metrics);
		mIPDx = 1 / metrics.xdpi;
		mIPDy = 1 / metrics.ydpi;
	}
	
	public float lengthInInches(float x, float y) {
		return (float) Math.hypot(x * mIPDx, y * mIPDy);
	}
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		super.onTouchEvent(event);
		if (mMainRenderer.mGame != null) {
			return mMainRenderer.mGame.onTouchEvent(this, event);
		}
		
		return false;
	}
}
