package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESBuffer;
import bg.alex_iii.GLES.GLESGeometry;
import bg.alex_iii.GLES.GLESMaterial;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESSorter;
import bg.alex_iii.GLES.GLESUtil.VertexPosNormUV;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Util;
import bg.alex_iii.GLES.Vec;

import java.lang.Math;
import java.util.ArrayList;

public class Terrain {
	public Game mGame;
	public GLESModel mModel;
	public int mSizeX, mSizeY;
	public float mGridSize, mTexCoordScale;
	public float[] mHeights;
	public float mMinHeight, mMaxHeight;
	
	public class LineTracer {
		float[] mOrigin, mDirection;
		float mMinFactor, mMaxFactor;
		float mFactor, mNextX, mNextY, mXFactor, mYFactor;
		
		LineTracer(float[] origin, float[] direction, float minFactor, float maxFactor) {
			mOrigin = origin;
			mDirection = direction;
			mMinFactor = minFactor;
			mMaxFactor = maxFactor;
			mFactor = minFactor;
			
			float extentX = getSizeX() / 2;
			float extentY = getSizeY() / 2;
			mNextX = ((int) ((mOrigin[0] + mFactor * mDirection[0] + extentX) / mGridSize) + (mDirection[0] > 0 ? 1 : 0)) * mGridSize - extentX; 
			mNextY = ((int) ((mOrigin[1] + mFactor * mDirection[1] + extentY) / mGridSize) + (mDirection[1] > 0 ? 1 : 0)) * mGridSize - extentY;
			mXFactor = Util.isZero(mDirection[0]) ? Float.POSITIVE_INFINITY : (mNextX - mOrigin[0]) / mDirection[0]; 
			mYFactor = Util.isZero(mDirection[1]) ? Float.POSITIVE_INFINITY : (mNextY - mOrigin[1]) / mDirection[1];
		}
		
		float nextFactor() {
			if (mFactor >= mMaxFactor || mXFactor == Float.POSITIVE_INFINITY && mYFactor == Float.POSITIVE_INFINITY)
				return Float.NaN;
			
			if (mXFactor < mYFactor) {
				mFactor = mXFactor;
				mNextX += Math.signum(mDirection[0]) * mGridSize;
				mXFactor = (mNextX - mOrigin[0]) / mDirection[0];
			} else {
				mFactor = mYFactor;
				mNextY += Math.signum(mDirection[1]) * mGridSize;
				mYFactor = (mNextY - mOrigin[1]) / mDirection[1];
			}
			
			return Math.min(mFactor, mMaxFactor);
		}
	}
	
	public Terrain(Game game, int sizeX, int sizeY, float gridSize, float texCoordScale) {
		mGame = game;
		mSizeX = sizeX;
		mSizeY = sizeY;
		mGridSize = gridSize;
		mTexCoordScale = texCoordScale;
		mMinHeight = Float.POSITIVE_INFINITY;
		mMaxHeight = Float.NEGATIVE_INFINITY;
	}
	
	public void initHeights() {
		mHeights = new float[mSizeX * mSizeY];
		float scaleX = (2 * (float) Math.PI) / mSizeX; 
		float scaleY = (2 * (float) Math.PI) / mSizeY;
		float scaleZ = Math.max(mSizeX, mSizeY) * mGridSize / 32;
		for (int y = 0; y < mSizeY; ++y)
			for (int x = 0; x < mSizeX; ++x) {
				setHeight(x, y, ((float) Math.sin(x * scaleX) + (float) Math.sin(y * scaleY)) * scaleZ);
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
		int x0, x1, y0, y1;
		x0 = Math.min(Math.max(0, x - 1), mSizeX - 1);
		x1 = Math.min(Math.max(0, x + 1), mSizeX - 1);
		y0 = Math.min(Math.max(0, y - 1), mSizeY - 1);
		y1 = Math.min(Math.max(0, y + 1), mSizeY - 1);
		float[] dx = Vec.get(x1 - x0, 0, mHeights[y * mSizeX + x1] - mHeights[y * mSizeX + x0]);
		float[] dy = Vec.get(0, y1 - y0, mHeights[y1 * mSizeX + x] - mHeights[y0 * mSizeX + x]);
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

		if (Util.isZero(direction[0]) && Util.isZero(direction[1])) {
			if (Util.isZero(direction[2]))
				return Float.NaN;
			float zTer = getHeight(origin[0], origin[1]);
			factor = (zTer - origin[2]) / direction[2];
			if (factor < factorMinMax[0] || factor > factorMinMax[1])
				return Float.NaN;
			return factor;
		}

		LineTracer tracer = new LineTracer(origin, direction, factor, factorMinMax[1]);
		
		float zTerrain = getHeight(origin[0] + factor * direction[0], origin[1] + factor * direction[1]);
		float z = origin[2] + factor * direction[2];
		while(!Float.isNaN(factor)) {
			float nextFactor = tracer.nextFactor();
			float nextZ = origin[2] + nextFactor * direction[2]; 
			float nextZTerrain = getHeight(origin[0] + nextFactor * direction[0], origin[1] + nextFactor * direction[1]);
			float dif = zTerrain - z;
			float nextDif = nextZTerrain - nextZ;
			if (Math.signum(dif) != Math.signum(nextDif)) {
				float a = dif / (dif - nextDif);
				float factorIntersection = (nextFactor - factor) * a + factor;
				return factorIntersection;
			}
			factor = nextFactor;
			zTerrain = nextZTerrain;
			z = nextZ;
		}
		
		return Float.NaN;
	}
	
	public void breakLine(float[] p0, float[] p1, ArrayList<Float> factors) {
		float[] direction = Vec.sub(p1, p0);
		float factor = 0;
		LineTracer tracer = new LineTracer(p0, direction, factor, 1);
		factors.clear();
		while (!Float.isNaN(factor)) {
			factors.add(factor);
			factor = tracer.nextFactor();
		}
	}
	
	public boolean initModel() {
		VertexPosNormUV[] vertices = new VertexPosNormUV[mSizeX * mSizeY];
		for (int y = 0; y < mSizeY; ++y)
			for (int x = 0; x < mSizeX; ++x) {
				float[] normal = getNormal(x, y);
				vertices[y * mSizeX + x] = new VertexPosNormUV(x * mGridSize, y * mGridSize, getHeight(x, y), 
																		 normal[0], normal[1], normal[2], 
																		 (float) x * mTexCoordScale / (mSizeX - 1), (float) y * mTexCoordScale / (mSizeY - 1));
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
		if (!vb.init(vertices, null))
			return false;
		ib = new GLESBuffer(true, GLESBuffer.Usage.STATIC_DRAW);
		if (!ib.init(indices, null))
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
	
	public boolean addToSorter(GLESSorter sorter) {
		return sorter.add(mModel);
	}
}
