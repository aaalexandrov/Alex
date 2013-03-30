package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESCamera;
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
import bg.alex_iii.GLES.Vec;

public class MainRenderer implements GLESUserRenderer {
	public GLESRenderer mRenderer;
	public MainActivity mActivity;
	public GLESModel mPrism, mSphere, mCone;
	public Game mGame;

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
	}

	public boolean init() {
		if (!initModels())
			return false;
		
		mGame = new Game(this);
		if (!mGame.init())
			return false;
		
		return true;
	}

	public void update() {
		mGame.update();
		final float fps = 1000.0f / (mGame.mTime - mGame.mTimePrev);
		mActivity.runOnUiThread(new Runnable() {
			public void run() {
				mActivity.setStatusText("FPS: " + fps);
			}
		});
	}

	public boolean render() {
		boolean result = true;

		//result &= mPrism.render();
		//result &= mSphere.render();
		result &= mGame.render();

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
		GLESShader shader = mRenderer.loadShader("tex_lit", R.raw.tex_lit_v,
				R.raw.tex_lit_f);
		if (shader == null)
			return false;
		GLESTexture texture = mRenderer.loadTexture("robot",
				GLESTexture.MinFilter.LINEAR, GLESTexture.MagFilter.LINEAR,
				GLESTexture.WrapMode.REPEAT, GLESTexture.WrapMode.REPEAT,
				R.raw.robot);
		if (texture == null)
			return false;
		GLESState state = new GLESState();
		GLESMaterial material = mRenderer.createMaterial("tex_lit", shader,
				state);
		if (material == null)
			return false;
		if (!material.setSampler("sTexture", texture))
			return false;
		material.setUniform("uLightDir", Vec.getNormalized(Vec.get(1, 1, 1)));
		material.setUniform("uLightDiffuse", Vec.get(1, 1, 1));
		material.setUniform("uLightAmbient", Vec.get(0.3f, 0.3f, 0.3f));

		material.setUniform("uMaterialDiffuse", Vec.get(0.7f, 0.7f, 0.7f));
		material.setUniform("uMaterialAmbient", Vec.get(1, 1, 1));

		shader = mRenderer.loadShader("color_lit", R.raw.color_lit_v,
				R.raw.color_lit_f);
		if (shader == null)
			return false;
		state = new GLESState();
		material = mRenderer.createMaterial("color_lit", shader, state);
		if (material == null)
			return false;
		material.setUniform("uLightDir", Vec.getNormalized(Vec.get(1, 1, 1)));
		material.setUniform("uLightDiffuse", Vec.get(1, 1, 1));
		material.setUniform("uLightAmbient", Vec.get(0.3f, 0.3f, 0.3f));

		material.setUniform("uMaterialDiffuse", Vec.get(0.7f, 0.7f, 0.0f));
		material.setUniform("uMaterialAmbient", Vec.get(1, 1, 0));
//		material.mState.setBackfaceCulling(false);
		
		shader = mRenderer.loadShader("color", R.raw.color_v, R.raw.color_f);
		if (shader == null)
			return false;
		state = new GLESState();
		material = mRenderer.createMaterial("color", shader, state);
		if (material == null)
			return false;
		material.setUniform("uColor", Vec.get(0, 1, 0, 1));

		return true;
	}

	protected boolean initModels() {
		if (!initMaterials())
			return false;

		mPrism = GLESUtil.createPrism(GameSettings.TARGET_RADIUS, 10, Vec.get(0, 0, GameSettings.TARGET_HEIGHT), true).createModel(mRenderer.mMaterials.get("color_lit"));
		mCone = GLESUtil.createPyramid(GameSettings.BASE_RADIUS, 10, Vec.get(0, 0, GameSettings.BASE_HEIGHT), true).createModel(mRenderer.mMaterials.get("color_lit"));
		mSphere = GLESUtil.createSphere(1, 2, false).createModel(mRenderer.mMaterials.get("color_lit"));
	
		return true;
	}

	protected boolean initCameraProjection(float aspect) {
		float near = 0.1f;
		float far = 100.0f;
		float fovFactor = 0.5f;
		mRenderer.mCamera.setProjection(-aspect * near * fovFactor, aspect * near * fovFactor, -1 * near * fovFactor,
				1 * near * fovFactor, near, far);
		return true;
	}
}
