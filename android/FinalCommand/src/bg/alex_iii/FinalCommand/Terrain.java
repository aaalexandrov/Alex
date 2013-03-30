package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESBuffer;
import bg.alex_iii.GLES.GLESGeometry;
import bg.alex_iii.GLES.GLESMaterial;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESUtil.VertexPosNormUV;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

import java.lang.Math;

public class Terrain {
	public Game mGame;
	public GLESModel mModel;
	public int mSizeX, mSizeY;
	public float mGridSize;
	public float[] mHeights;
	public float mMinHeight, mMaxHeight;
	
	public Terrain(Game game, int sizeX, int sizeY, float gridSize) {
		mGame = game;
		mSizeX = sizeX;
		mSizeY = sizeY;
		mGridSize = gridSize;
		mMinHeight = Float.POSITIVE_INFINITY;
		mMaxHeight = Float.NEGATIVE_INFINITY;
	}
	
	public void initHeights() {
		mHeights = new float[mSizeX * mSizeY];
		float scaleX = (2 * (float) Math.PI) / mSizeX; 
		float scaleY = (2 * (float) Math.PI) / mSizeY;
		float scaleZ = Math.max(mSizeX, mSizeY) * mGridSize / 8;
		for (int y = 0; y < mSizeY; ++y)
			for (int x = 0; x < mSizeX; ++x) {
				mHeights[y * mSizeX + x] = ((float) Math.sin(x * scaleX) + (float) Math.sin(y * scaleY)) * scaleZ;
			}
	}
	
	public float getSizeX() {
		return (mSizeX - 1) * mGridSize;
	}

	public float getSizeY() {
		return (mSizeY - 1) * mGridSize;
	}
	
	public float getHeight(float x, float y) {
		x /= mGridSize;
		y /= mGridSize;
		x += (mSizeX - 1) * 0.5f;
		y += (mSizeY - 1) * 0.5f;
		float h00 = getHeight((int) x, (int) y);
		float h01 = getHeight((int) x + 1, (int) y);
		float h10 = getHeight((int) x, (int) y + 1);
		float h11 = getHeight((int) x + 1, (int) y + 1);
		float wx = x - (int) x;
		float wy = y - (int) y;
		float h0 = (h01 - h00) * wx + h00;
		float h1 = (h11 - h10) * wx + h10;
		float h = (h1 - h0) * wy + h0;
		return h;
	}
	
	public float getHeight(int x, int y) {
		x = Math.min(Math.max(0, x), mSizeX - 1);
		y = Math.min(Math.max(0, y), mSizeY - 1);
		return mHeights[y * mSizeX + x];
	}
	
	public float[] getNormal(int x, int y) {
		float[] dx = Vec.get(2, 0, getHeight(x + 1, y) - getHeight(x - 1, y));
		float[] dy = Vec.get(0, 2, getHeight(x, y + 1) - getHeight(x, y - 1));
		float[] n = Vec.cross(dx, dy);
		return Vec.normalize(n);
	}
	
	public void setHeight(int x, int y, float h) {
		mHeights[y * mSizeY + x] = h;
		if (h < mMinHeight)
			mMinHeight = h;
		if (h > mMaxHeight)
			mMaxHeight = h;
	}
	
	public float getRayIntersection(float[] origin, float[] direction) {
		float extentX = getSizeX() / 2;
		float extentY = getSizeY() / 2;
		float[] boxMin = Vec.get(-extentX, -extentY, mMinHeight);
		float[] boxMax = Vec.get(extentX, extentY, mMaxHeight);
		float[] factorMinMax = new float[2];
		
		if (!Shape.isRayIntersectingBox(boxMin, boxMax, origin, direction, factorMinMax))
			return Float.NaN;
		if (factorMinMax[1] < 0)
			return Float.NaN;
		float factor = Math.max(0, factorMinMax[0]);
		
		while(factor < factorMinMax[1]) {
			
		}
		
		return Float.NaN;
	}
	
	public boolean initModel() {
		VertexPosNormUV[] vertices = new VertexPosNormUV[mSizeX * mSizeY];
		for (int y = 0; y < mSizeY; ++y)
			for (int x = 0; x < mSizeX; ++x) {
				float[] normal = getNormal(x, y);
				vertices[y * mSizeX + x] = new VertexPosNormUV(x * mGridSize, y * mGridSize, getHeight(x, y), 
																		 normal[0], normal[1], normal[2], 
																		 (float) x / (mSizeX - 1), (float) y / (mSizeY - 1));
			}
		short[] indices = new short[(mSizeX - 1) * (mSizeY - 1) * 6];
		for (int y = 0; y < mSizeY - 1; ++y)
			for (int x = 0; x < mSizeX - 1; ++x) {
				int baseVert = y * mSizeX + x;
				int baseInd = (y * (mSizeX - 1) + x) * 6;
				indices[baseInd + 0] = (short) baseVert;
				indices[baseInd + 1] = (short) (baseVert + 1);
				indices[baseInd + 2] = (short) (baseVert + mSizeX);
				indices[baseInd + 3] = (short) (baseVert + 1);
				indices[baseInd + 4] = (short) (baseVert + mSizeX + 1);
				indices[baseInd + 5] = (short) (baseVert + mSizeX);
			}
		
		GLESBuffer vb, ib;
		vb = new GLESBuffer(false, GLESBuffer.Usage.STATIC_DRAW);
		if (!vb.init(vertices))
			return false;
		ib = new GLESBuffer(true, GLESBuffer.Usage.STATIC_DRAW);
		if (!ib.init(indices))
			return false;
		GLESGeometry geometry = new GLESGeometry(vb, ib,
				GLESGeometry.PrimitiveType.TRIANGLES);
		float[] transform = new float[16];
		Matrix.setIdentityM(transform, 0);
		Matrix.translateM(transform, 0, -(mSizeX - 1) * mGridSize / 2, -(mSizeY - 1) * mGridSize / 2, 0);
		GLESMaterial material = mGame.mMainRenderer.mRenderer.mMaterials.get("tex_lit");
		mModel = new GLESModel(geometry, material, transform);
		return true;
	}
	
	public boolean init() {
		initHeights();
		return initModel();
	}
	
	public boolean render() {
		return mModel.render();
	}
}
