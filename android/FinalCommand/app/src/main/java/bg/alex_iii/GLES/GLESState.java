package bg.alex_iii.GLES;

import android.opengl.GLES20;

public class GLESState {
	public static final String TAG = "GLESState";
	
	public enum BlendMode {
		COPY, BLEND, ADD, 
	}
	
	public enum CompareFunc {
		NEVER, LT, LTEQ, EQ, NOTEQ, GTEQ, GT, ALWAYS, 
	}
	
	public enum Face {
		FRONT, BACK, FRONT_AND_BACK,
	}
	
	public enum StencilOp {
		KEEP, ZERO, REPLACE, INCR, INCR_WRAP, DECR, DECR_WRAP, INVERT,
	}
	
	public static final int MASK_ONES = Integer.MAX_VALUE | Integer.MIN_VALUE;
	
	protected BlendMode mBlendMode = BlendMode.COPY;
	
	protected boolean mDepthEnable = true;
	protected CompareFunc mDepthFunc = CompareFunc.LT;
	protected boolean mDepthWrite = true;
	protected float mDepthBias = 0.0f;
	protected float mDepthBiasScale = 0.0f;
	
	protected boolean mBackfaceCulling = true; 
	
	protected boolean mStencilEnable = false; 
	protected CompareFunc[] mStencilFunc = { CompareFunc.ALWAYS, CompareFunc.ALWAYS };
	protected int[] mStencilRef = { MASK_ONES, MASK_ONES }; 
	protected int[] mStencilMask = { MASK_ONES, MASK_ONES };
	protected int[] mStencilWriteMask = { MASK_ONES, MASK_ONES };
	protected StencilOp[] mStencilOpFail = { StencilOp.KEEP, StencilOp.KEEP };
	protected StencilOp[] mStencilOpDepthFail = { StencilOp.KEEP, StencilOp.KEEP };
	protected StencilOp[] mStencilOpPass = { StencilOp.KEEP, StencilOp.KEEP };

	public GLESState() {
	}
	
	public void copyFrom(GLESState state) {
		mBlendMode = state.mBlendMode;
		
		mDepthEnable = state.mDepthEnable;
		mDepthFunc = state.mDepthFunc;
		mDepthWrite = state.mDepthWrite;
		mDepthBias = state.mDepthBias;
		mDepthBiasScale = state.mDepthBiasScale;
		
		mBackfaceCulling = state.mBackfaceCulling;
		
		mStencilEnable = state.mStencilEnable;
		mStencilFunc = state.mStencilFunc;
		mStencilRef = state.mStencilRef;
		mStencilMask = state.mStencilMask;
		mStencilWriteMask = state.mStencilWriteMask;
		mStencilOpFail = state.mStencilOpFail;
		mStencilOpDepthFail = state.mStencilOpDepthFail;
		mStencilOpPass = state.mStencilOpPass;
	}
	
	public BlendMode getBlendMode() {
		return mBlendMode;
	}
	
	public void setBlendMode(BlendMode blendMode) {
		mBlendMode = blendMode;
	}

	protected boolean applyBlend() {
		switch (mBlendMode) {
			case COPY:
				GLES20.glDisable(GLES20.GL_BLEND);
				break;
			case BLEND:
				GLES20.glBlendEquation(GLES20.GL_FUNC_ADD);
				GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE_MINUS_SRC_ALPHA);
				GLES20.glEnable(GLES20.GL_BLEND);
				break;
			case ADD:
				GLES20.glBlendEquation(GLES20.GL_FUNC_ADD);
				GLES20.glBlendFunc(GLES20.GL_SRC_ALPHA, GLES20.GL_ONE);
				GLES20.glEnable(GLES20.GL_BLEND);
				break;
		}
		return true;
	}
	
	public boolean getDepthEnable() {
		return mDepthEnable;
	}
	
	public void setDepthEnable(boolean depthEnable) {
		mDepthEnable = depthEnable;
	}
	
	public CompareFunc getDepthFunc() {
		return mDepthFunc;
	}

	public void setDepthMode(CompareFunc depthFunc) {
		mDepthFunc = depthFunc;
	}

	public boolean getDepthWrite() {
		return mDepthWrite;
	}

	public void setDepthWrite(boolean depthWrite) {
		mDepthWrite = depthWrite;
	}

	public float getDepthBias() {
		return mDepthBias;
	}
	
	public float getDepthBiasScale() {
		return mDepthBiasScale;
	}
	
	public void setDepthBias(float bias, float scale) {
		mDepthBias = bias;
		mDepthBiasScale = scale;
	}
	
	protected boolean applyDepth() {
		if (!mDepthEnable) {
			GLES20.glDisable(GLES20.GL_DEPTH_TEST);
			return true;
		}
		GLES20.glEnable(GLES20.GL_DEPTH_TEST);
		GLES20.glDepthFunc(compareFunc2GL(mDepthFunc));
		GLES20.glDepthMask(mDepthWrite);
		if (mDepthBias != 0.0 || mDepthBiasScale != 0.0) {
			GLES20.glPolygonOffset(mDepthBiasScale, mDepthBias);
			GLES20.glEnable(GLES20.GL_POLYGON_OFFSET_FILL);
		} else
			GLES20.glDisable(GLES20.GL_POLYGON_OFFSET_FILL);
		return true;
	}
	
	public boolean getBackfaceCulling() {
		return mBackfaceCulling;
	}
	
	public void setBackfaceCulling(boolean enableCulling) {
		mBackfaceCulling = enableCulling;
	}
	
	protected boolean applyCulling() {
		if (mBackfaceCulling) {
			GLES20.glFrontFace(GLES20.GL_CCW);
			GLES20.glCullFace(GLES20.GL_BACK);
			GLES20.glEnable(GLES20.GL_CULL_FACE);
		} else 
			GLES20.glDisable(GLES20.GL_CULL_FACE);
		return true;
	}
	
	public boolean getStencilEnable() {
		return mStencilEnable;
	}

	public void setStencilEnable(boolean stencilEnable) {
		mStencilEnable = stencilEnable;
	}

	public CompareFunc getStencilFunc(Face face) {
		if (face == Face.FRONT_AND_BACK)
			face = Face.FRONT;
		return mStencilFunc[face.ordinal()];
	}

	public int getStencilRef(Face face) {
		if (face == Face.FRONT_AND_BACK)
			face = Face.FRONT;
		return mStencilRef[face.ordinal()];
	}

	public int getStencilMask(Face face) {
		if (face == Face.FRONT_AND_BACK)
			face = Face.FRONT;
		return mStencilMask[face.ordinal()];
	}
	
	public void setStencilFunc(Face face, CompareFunc stencilFunc, int stencilRef, int stencilMask) {
		if (face == Face.FRONT_AND_BACK) {
			setStencilFunc(Face.BACK, stencilFunc, stencilRef, stencilMask);
			face = Face.FRONT;
		}
		mStencilFunc[face.ordinal()] = stencilFunc;
		mStencilRef[face.ordinal()] = stencilRef;
		mStencilMask[face.ordinal()] = stencilMask;
	}
	
	public int getStencilWriteMask(Face face) {
		if (face == Face.FRONT_AND_BACK)
			face = Face.FRONT;
		return mStencilWriteMask[face.ordinal()];
	}
	
	public void setStencilWriteMask(Face face, int stencilWriteMask) {
		if (face == Face.FRONT_AND_BACK) {
			setStencilWriteMask(Face.BACK, stencilWriteMask);
			face = Face.FRONT;
		}
		mStencilWriteMask[face.ordinal()] = stencilWriteMask;
	}
	
	public void setStencilOp(Face face, StencilOp opFail, StencilOp opDepthFail, StencilOp opPass) {
		if (face == Face.FRONT_AND_BACK) {
			setStencilOp(Face.BACK, opFail, opDepthFail, opPass);
			face = Face.FRONT;
		}
		mStencilOpFail[face.ordinal()] = opFail;
		mStencilOpDepthFail[face.ordinal()] = opDepthFail;
		mStencilOpPass[face.ordinal()] = opPass;
	}
	
	protected boolean applyStencil() {
		if (!mStencilEnable) {
			GLES20.glDisable(GLES20.GL_STENCIL_TEST);
			return true;
		}
		GLES20.glEnable(GLES20.GL_STENCIL_TEST);
		for (int i = 0; i < 2; ++i) {
			GLES20.glStencilFuncSeparate(ordinal2GLFace(i), compareFunc2GL(mStencilFunc[i]), mStencilRef[i], mStencilMask[i]);
			GLES20.glStencilMaskSeparate(ordinal2GLFace(i), mStencilWriteMask[i]);
			GLES20.glStencilOpSeparate(ordinal2GLFace(i), stencilOp2GL(mStencilOpFail[i]), stencilOp2GL(mStencilOpDepthFail[i]), stencilOp2GL(mStencilOpPass[i]));
		}
		return true;
	}
	
	public boolean apply() {
		if (!applyBlend())
			return false;
		if (!applyDepth())
			return false;
		if (!applyCulling())
			return false;
		if (!applyStencil())
			return false;
		return true;
	}
	
	protected int compareFunc2GL(CompareFunc func) {
		switch (func) {
			case LT:
				return GLES20.GL_LESS;
			case LTEQ:
				return GLES20.GL_LEQUAL;
			case EQ:
				return GLES20.GL_EQUAL;
			case NOTEQ:
				return GLES20.GL_NOTEQUAL;
			case GTEQ:
				return GLES20.GL_GEQUAL;
			case GT:
				return GLES20.GL_GREATER;
			case NEVER:
				return GLES20.GL_NEVER;
			case ALWAYS:
			default:
				return GLES20.GL_ALWAYS;
		}
	}
	
	protected int stencilOp2GL(StencilOp stencilOp) {
		switch (stencilOp) {
			case ZERO: 
				return GLES20.GL_ZERO;
			case REPLACE:
				return GLES20.GL_REPLACE;
			case INCR:
				return GLES20.GL_INCR;
			case INCR_WRAP:
				return GLES20.GL_INCR_WRAP;
			case DECR:
				return GLES20.GL_DECR;
			case DECR_WRAP:
				return GLES20.GL_DECR_WRAP;
			case INVERT:
				return GLES20.GL_INVERT;
			case KEEP:
			default:
				return GLES20.GL_KEEP;
		}
	}
	
	protected int ordinal2GLFace(int i) {
		if (i == 0)
			return GLES20.GL_FRONT;
		return GLES20.GL_BACK;
	}
}
