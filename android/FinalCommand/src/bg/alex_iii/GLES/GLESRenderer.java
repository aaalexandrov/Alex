package bg.alex_iii.GLES;

import java.util.HashMap;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.util.Log;

public class GLESRenderer implements Renderer {
	public static final String TAG = "GLESRenderer";
	
	public Activity mContext;
	public HashMap<String, GLESShader> mShaders = new HashMap<String, GLESShader>();
	public HashMap<String, GLESTexture> mTextures = new HashMap<String, GLESTexture>();
	public HashMap<String, GLESMaterial> mMaterials = new HashMap<String, GLESMaterial>();
	public GLESCamera mCamera;
	public long mFrameID;
	public float[] mClearColor = { 0, 0, 1, 1 };
	public float mClearDepth = 1;
	public int mClearStencil = 0; 
	
	public GLESUserRenderer mUserRenderer;
	
	public GLESRenderer(Context context, GLSurfaceView surfaceView, GLESUserRenderer userRenderer) {
		mContext = (Activity) context;
		mFrameID = 1;
		mUserRenderer = userRenderer;
		mUserRenderer.setRenderer(this);
		surfaceView.setEGLContextClientVersion(2);
		surfaceView.setRenderer(this);
	}

	public GLESShader loadShader(String name, int vertexSrcResource, int fragmentSrcResource) {
		GLESShader shader = mShaders.get(name);
		if (shader != null)
			return shader;
		shader = new GLESShader(this, name);
		if (!shader.init(mContext, vertexSrcResource, fragmentSrcResource))
			return null;
		mShaders.put(shader.mName, shader);
		return shader;
	}
	
	public GLESTexture loadTexture(String name, GLESTexture.MinFilter minFilter, GLESTexture.MagFilter magFilter, GLESTexture.WrapMode wrapS, GLESTexture.WrapMode wrapT, int texResource) {
		GLESTexture texture = mTextures.get(name);
		if (texture != null)
			return texture;
		texture = new GLESTexture(name);
		if (!texture.init(minFilter, magFilter, wrapS, wrapT, mContext, texResource))
			return null;
		mTextures.put(texture.mName, texture);
		return texture;
	}
	
	public GLESMaterial createMaterial(String name, GLESShader shader, GLESState state) {
		GLESMaterial material = mMaterials.get(name);
		if (material != null) {
			assert(shader == material.mShader);
			assert(state == material.mState);
			return material;
		}
		material = new GLESMaterial(name, shader, state);
		mMaterials.put(material.mName, material);
		return material;
	}
	
	public void setCamera(GLESCamera camera) {
		mCamera = camera;
	}
	
	public void onDrawFrame(GL10 gl) {
		mUserRenderer.update();
		
		GLES20.glDepthRangef(0, 1);
		GLES20.glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
		GLES20.glClearDepthf(mClearDepth);
		GLES20.glClearStencil(mClearStencil);
		GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT | GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_STENCIL_BUFFER_BIT);

		if (!mUserRenderer.render())
			Log.e(TAG, "User render failed");
		
		mFrameID++;
	}

	public void onSurfaceChanged(GL10 gl, int width, int height) {
		GLES20.glViewport(0, 0, width, height);
		mUserRenderer.setDimensions(width, height);
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		mUserRenderer.init();
	}
}
