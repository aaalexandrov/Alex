package bg.alex_iii.FinalCommand;

import java.util.Comparator;

import android.graphics.Typeface;
import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESCamera;
import bg.alex_iii.GLES.GLESCompareSorter;
import bg.alex_iii.GLES.GLESFont;
import bg.alex_iii.GLES.GLESGeometry;
import bg.alex_iii.GLES.GLESMaterial;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESRenderer;
import bg.alex_iii.GLES.GLESShader;
import bg.alex_iii.GLES.GLESState;
import bg.alex_iii.GLES.GLESTexture;
import bg.alex_iii.GLES.GLESUserRenderer;
import bg.alex_iii.GLES.GLESUtil;
import bg.alex_iii.GLES.GLESUtil.VertexPos;
import bg.alex_iii.GLES.GLESUtil.VertexPosNormUV;
import bg.alex_iii.GLES.SoundPlayer;
import bg.alex_iii.GLES.Vec;

public class MainRenderer implements GLESUserRenderer {
	public static final float FOV_DEGREES = 60;
	
	public GLESRenderer mRenderer;
	public MainActivity mActivity;
	public GLESModel mPrism, mSphere, mCone;
	public Game mGame;
	public GLESCompareSorter mSorter;
	public GLESFont mFont;
	public LineHolder mLineHolder;
	public SoundPlayer mSoundPlayer;
	GLESModel mFontTest;

	public void setRenderer(GLESRenderer renderer) {
		mRenderer = renderer;
		mRenderer.setCamera(new GLESCamera());
		mActivity = (MainActivity) mRenderer.mContext;
	}

	public GLESRenderer getRenderer() {
		return mRenderer;
	}

	public void setDimensions(int width, int height) {
		initCameraProjection((float) width / height);
		initFontModel(width, height);
	}

	public boolean init() {
		mSorter = new GLESCompareSorter(new Comparator<GLESModel>() {
			public int compare(GLESModel model0, GLESModel model1) {
				boolean blend0, blend1;
				blend0 = model0.mMaterial.mState.getBlendMode() != GLESState.BlendMode.COPY;
				blend1 = model1.mMaterial.mState.getBlendMode() != GLESState.BlendMode.COPY;
				if (blend0 && !blend1)
					return 1;
				if (blend1 && !blend0)
					return -1;
				return 0;
			}
		});
		
		if (!initModels())
			return false;

		mLineHolder = new LineHolder(this.mRenderer);
		
		if (!initSound()) 
			return false;
		
		mGame = new Game(this);
		if (!mGame.init())
			return false;
		
		return true;
	}

	public void update() {
		mLineHolder.clearPoints();
		mGame.update();
		final float fps = 1000.0f / (mGame.mTime - mGame.mTimePrev);
		mActivity.runOnUiThread(new Runnable() {
			public void run() {
				mActivity.setStatusText(String.format("FPS: %.2f",  fps));
			}
		});
		mLineHolder.updateModel();
	}

	public boolean render() {
		boolean result = true;

		result &= mGame.render();
		
		result &= mLineHolder.addToSorter(mSorter);

		result &= mSorter.sortAndRender();

		if (mFontTest != null)
			result &= mFontTest.render();
		
		return result;
	}

	protected GLESModel createQuad() {
		VertexPosNormUV[] vertices = {
				new VertexPosNormUV(-0.5f, -0.5f, 0.5f, 0, 0, -1, 0, 0),
				new VertexPosNormUV(-0.5f, 0.5f, 0.5f, 0, 0, -1, 0, 1),
				new VertexPosNormUV(0.5f, 0.5f, 0.5f, 0, 0, -1, 1, 1),
				new VertexPosNormUV(0.5f, -0.5f, 0.5f, 0, 0, -1, 1, 0), };
		short[] indices = { 0, 2, 1, 2, 0, 3 };
		return GLESModel.create(vertices, indices, GLESGeometry.PrimitiveType.TRIANGLES, mRenderer.mMaterials.get("tex_lit"));
	}

	protected GLESModel initLine() {
		VertexPos[] vertices = { new VertexPos(0, 0, 0),
				new VertexPos(0, 0, 1), };
		short[] indices = { 0, 1 };
		return GLESModel.create(vertices, indices, GLESGeometry.PrimitiveType.LINES, mRenderer.mMaterials.get("color"));
	}

	protected GLESModel initCube() {
		VertexPosNormUV[] vertices = new VertexPosNormUV[24];
		short[] indices = new short[36];
		int vert = 0, ind = 0;
		int dim, x, y, z, bias;
		for (dim = 0; dim < 3; dim++)
			for (z = -1; z <= 1; z += 2) {
				for (y = -1; y <= 1; y += 2)
					for (x = -1; x <= 1; x += 2) {
						float[] pos = new float[3];
						pos[dim] = x;
						pos[(dim + 1) % 3] = y;
						pos[(dim + 2) % 3] = z;
						float[] norm = new float[3];
						norm[dim] = 0;
						norm[(dim + 1) % 3] = 0;
						norm[(dim + 2) % 3] = z;
						float u, v;
						u = x > 0 ? 1 : 0;
						v = y > 0 ? 1 : 0;
						vertices[vert++] = new VertexPosNormUV(pos[0], pos[1],
								pos[2], norm[0], norm[1], norm[2], u, v);
					}
				if (z > 0)
					bias = 0;
				else
					bias = 1;
				indices[ind++] = (short) (vert - 4);
				indices[ind++] = (short) (vert - 3 + bias);
				indices[ind++] = (short) (vert - 2 - bias);

				indices[ind++] = (short) (vert - 2 - bias);
				indices[ind++] = (short) (vert - 3 + bias);
				indices[ind++] = (short) (vert - 1);
			}

		return GLESModel.create(vertices, indices, GLESGeometry.PrimitiveType.TRIANGLES, mRenderer.mMaterials.get("tex_lit"));
	}

	protected boolean initMaterials() {
		GLESShader shader = mRenderer.loadShader("tex_lit", R.raw.tex_lit_v, R.raw.tex_lit_f);
		if (shader == null)
			return false;
		GLESTexture texture = mRenderer.loadTexture("green_grid",
				GLESTexture.MinFilter.LINEAR_MIPMAP_LINEAR, GLESTexture.MagFilter.LINEAR,
				GLESTexture.WrapMode.REPEAT, GLESTexture.WrapMode.REPEAT,
				R.raw.green_grid);
		if (texture == null)
			return false;
		GLESState state = new GLESState();
		GLESMaterial material = mRenderer.createMaterial("tex_lit", shader, state);
		if (material == null)
			return false;
		if (!material.setSampler("sTexture", texture))
			return false;
		material.setUniform("uLightDir", Vec.getNormalized(Vec.get(1, 1, 1)));
		material.setUniform("uLightDiffuse", Vec.get(1, 1, 1));
		material.setUniform("uLightAmbient", Vec.get(0.3f, 0.3f, 0.3f));

		material.setUniform("uMaterialDiffuse", Vec.get(0.7f, 0.7f, 0.7f));
		material.setUniform("uMaterialAmbient", Vec.get(1, 1, 1, 1));

		shader = mRenderer.loadShader("tex", R.raw.tex_v, R.raw.tex_f);
		if (shader == null)
			return false;
		texture = mFont.mTexture;
		if (texture == null)
			return false;
		state = new GLESState();
		state.setBackfaceCulling(false);
		state.setDepthEnable(false);
		state.setBlendMode(GLESState.BlendMode.BLEND);
		material = mRenderer.createMaterial("tex", shader, state);
		if (material == null)
			return false;
		if (!material.setSampler("sTexture", texture))
			return false;
		material.setUniform("uColor", Vec.get(1, 1, 1, 1));
		float[] transform = new float[16];
		Matrix.setIdentityM(transform, 0);
		material.setUniform("umTransform", transform);
		
		shader = mRenderer.loadShader("color_lit", R.raw.color_lit_v, R.raw.color_lit_f);
		if (shader == null)
			return false;
		state = new GLESState();
//		state.setBackfaceCulling(false);
		material = mRenderer.createMaterial("color_lit", shader, state);
		if (material == null)
			return false;
		material.setUniform("uLightDir", Vec.getNormalized(Vec.get(1, 1, 1)));
		material.setUniform("uLightDiffuse", Vec.get(1, 1, 1));
		material.setUniform("uLightAmbient", Vec.get(0.3f, 0.3f, 0.3f));

		material.setUniform("uMaterialDiffuse", Vec.get(0.7f, 0.7f, 0.0f));
		material.setUniform("uMaterialAmbient", Vec.get(1, 1, 0, 1));
		
		shader = mRenderer.loadShader("color", R.raw.color_v, R.raw.color_f);
		if (shader == null)
			return false;
		state = new GLESState();
		material = mRenderer.createMaterial("color", shader, state);
		if (material == null)
			return false;
		material.setUniform("uColor", Vec.get(1, 1, 1, 1));

		shader = mRenderer.loadShader("color_vert", R.raw.color_vert_v, R.raw.color_vert_f);
		if (shader == null)
			return false;
		state = new GLESState();
		material = mRenderer.createMaterial("color_vert", shader, state);
		if (material == null)
			return false;
		
		return true;
	}

	protected boolean initFontModel(float viewportWidth, float viewportHeight) {
		String text = "Lorem ipsum 123";
		mFontTest = GLESModel.create(mFont.createVertices(text, 50, 50, viewportWidth, viewportHeight), mFont.createIndices(text, 0), GLESGeometry.PrimitiveType.TRIANGLES, mRenderer.mMaterials.get("tex"));

		return true;
	}
	
	protected boolean initModels() {
		mFont = new GLESFont("Monospace_20", 20, Typeface.NORMAL);
		if (!mFont.init())
			return false;
		
		if (!initMaterials())
			return false;

		mPrism = GLESUtil.createPrism(GameSettings.TARGET_RADIUS, 10, Vec.get(0, 0, GameSettings.TARGET_HEIGHT), true).createModel(mRenderer.mMaterials.get("color_lit"));
		mCone = GLESUtil.createPyramid(GameSettings.BASE_RADIUS, 10, Vec.get(0, 0, GameSettings.BASE_HEIGHT), false).createModel(mRenderer.mMaterials.get("color_lit"));
		mSphere = GLESUtil.createSphere(1, 2, false).createModel(mRenderer.mMaterials.get("color"));
	
		return true;
	}

	protected boolean initSound() {
		mSoundPlayer = new SoundPlayer(mRenderer.mContext, 8);
		
		mSoundPlayer.loadSound("explosion", R.raw.bomb_exploding, SoundPlayer.DEFAULT_PRIORITY);
		
		return true;
	}
	
	protected boolean initCameraProjection(float aspect) {
		float near = 0.5f;
		float far = 100.0f;
		
		float extentX, extentY;
		if (aspect >= 1) {
			extentY = (float) Math.tan(FOV_DEGREES) * near;
			extentX = extentY * aspect; 
		} else {
			extentX = (float) Math.tan(FOV_DEGREES) * near;
			extentY = extentX / aspect;
		}
		
		mRenderer.mCamera.setProjection(-extentX, extentX, -extentY, extentY, near, far);
		return true;
	}
}
