package bg.alex_iii.GLES;

import android.util.Log;
import bg.alex_iii.GLES.GLESShader.ParamSource;
import bg.alex_iii.GLES.GLESShader.UniformInfo;

public class GLESMaterial {
	public static final String TAG = "GLESMaterial";

	public static class Uniform {
		public int mIndexInShader;
		float[] mValue;
		
		Uniform(int indexInShader) {
			mIndexInShader = indexInShader;
		}
	}
	
	public GLESShader mShader;
	public Uniform[] mUniforms;
	public GLESTexture[] mSamplers;
	public GLESState mState;
	public String mName;
	
	
	public GLESMaterial(String name, GLESShader shader, GLESState state) {
		mShader = shader;
		mName = name;
		mState = state;
		initParameters();
	}
	
	public boolean initParameters() {
		int count;
		count = mShader.mUniforms.getParamCount(ParamSource.PER_MATERIAL);
		mUniforms = new Uniform[count];
		int i, uniInd = 0;
		for (i = 0; i < mShader.mUniforms.mParams.size(); i++) 
			if (mShader.mUniforms.getParam(i).getSource() == ParamSource.PER_MATERIAL)
				mUniforms[uniInd++] = new Uniform(i);
		count = mShader.mSamplers.getParamCount();
		mSamplers = new GLESTexture[count];
		return true;
	}
	
	public int getUniformIndex(String name) {
		for (int i = 0; i < mUniforms.length; i++)
			if (mShader.mUniforms.getParam(mUniforms[i].mIndexInShader).mName.equals(name))
				return i;
		return -1;
	}
	
	public int getSamplerIndex(String name) {
		return mShader.mSamplers.getParamIndex(name);
	}
	
	public boolean setUniform(int index, float[] value) {
		mUniforms[index].mValue = value;
		UniformInfo uni = mShader.mUniforms.getParam(mUniforms[index].mIndexInShader);
		if (value.length != uni.getSize() * Byte.SIZE / Float.SIZE) {
			Log.e(TAG, "Invalid value for parameter " + uni.mName + " in material " + mName);
			return false;
		}
		return true;
	}
	
	public boolean setUniform(String name, float[] value) {
		int index = getUniformIndex(name);
		if (index < 0)
			return false;
		return setUniform(index, value);
	}
	
	public boolean setSampler(int index, GLESTexture texture) {
		mSamplers[index] = texture;
		return texture != null;
	}

	public boolean setSampler(String name, GLESTexture texture) {
		int index = getSamplerIndex(name);
		if (index < 0)
			return false;
		return setSampler(index, texture);
	}
	
	public boolean apply(GLESModel model) {
		int i;
		boolean result = true;
		if (mState != null && !mState.apply())
			return false;
		if (!mShader.apply())
			return false;
		result &= mShader.setPerFrame(model);
		result &= mShader.setPerInstance(model);
		for (i = 0; i < mUniforms.length; i++) 
			result &= mShader.mUniforms.getParam(mUniforms[i].mIndexInShader).apply(mUniforms[i].mValue);
		for (i = 0; i < mSamplers.length; i++)
			result &= mShader.mSamplers.getParam(i).apply(mSamplers[i]);
		return result;
	}
}
