package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESRenderer;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.content.Context;

public class MainView extends GLSurfaceView {

	public GLESRenderer mGLESRenderer;
	public MainRenderer mMainRenderer;
	
	public MainView(Context context, AttributeSet attrs) {
		super(context, attrs);
		mMainRenderer = new MainRenderer();
		mGLESRenderer = new GLESRenderer(context, this, mMainRenderer);
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
