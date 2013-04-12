package bg.alex_iii.GLES;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringWriter;
import java.io.Writer;
import java.util.ArrayList;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.util.Log;

public class GLESShader {
	public static final String TAG = "GLESShader";
	
	enum ParamSource {
		UNKNOWN,
		PER_MATERIAL,
		PER_INSTANCE,
		PER_FRAME,
	}
	
	static public class ParamInfo {
		public int mId;
		public int mType;
		public int mElements;
		public String mName;
		
		public ParamInfo(int id, int type, int elements, String name) {
			mId = id;
			mType = type;
			mElements = elements;
			mName = name;
		}
		
		public int getSize() {
			return GLESShader.getTypeSize(mType, mElements);
		}
		
		public ParamSource getSource() {
			return ParamSource.UNKNOWN;
		}
	}
	
	public static interface ParamSetter {
		public boolean set(UniformInfo param, GLESModel model);
	}
	
	public static class SourcedParamInfo extends ParamInfo {
		ParamSource mSource;
		ParamSetter mSetter;
		
		public SourcedParamInfo(int id, int type, int elements, String name, GLESShader shader) {
			super(id, type, elements, name);
			setSourceAndSetter(shader);
		}
		
		@Override
		public ParamSource getSource() {
			return mSource;
		}
		
		public void setSourceAndSetter(final GLESShader shader) {
			if (mName.equals("umModel")) {
				mSource = ParamSource.PER_INSTANCE;
				mSetter = new ParamSetter() {
					public boolean set(UniformInfo param, GLESModel model) {
						return param.apply(model.mTransform);
					}
				};
				return;
			}
			if (mName.equals("umModelViewProj")) {
				mSource = ParamSource.PER_INSTANCE;
				mSetter = new ParamSetter() {
					public boolean set(UniformInfo param, GLESModel model) {
						float[] modelViewProj = new float[16];
						GLESCamera camera = shader.mRenderer.mCamera;
						float[] viewProj = camera.getViewProj(); 
						Matrix.multiplyMM(modelViewProj, 0, viewProj, 0, model.mTransform, 0);
						return param.apply(modelViewProj);
					}
				};
				return;
			}
			if (mName.equals("umView")) {
				mSource = ParamSource.PER_FRAME;
				mSetter = new ParamSetter() {
					public boolean set(UniformInfo param, GLESModel model) {
						return param.apply(shader.mRenderer.mCamera.mView);
					}
				};
				return;
			}
			if (mName.equals("umProj")) {
				mSource = ParamSource.PER_FRAME;
				mSetter = new ParamSetter() {
					public boolean set(UniformInfo param, GLESModel model) {
						return param.apply(shader.mRenderer.mCamera.mProj);
					}
				};
				return;
			}
			mSource = ParamSource.PER_MATERIAL;
		}
	}
	
	public static class UniformInfo extends SourcedParamInfo {
		public UniformInfo(int id, int type, int elements, String name, GLESShader shader) {
			super(id, type, elements, name, shader);
		}
		
		public boolean apply(float[] val) {
			if (val == null || val.length != getSize() * Byte.SIZE / Float.SIZE) {
				Log.e(TAG, "Invalid value for shader parameter " + mName);
				return false;
			}
			switch (mType) {
				case GLES20.GL_FLOAT:
					GLES20.glUniform1fv(mId, mElements, val, 0);
					break;
				case GLES20.GL_FLOAT_VEC2:
					GLES20.glUniform2fv(mId, mElements, val, 0);
					break;
				case GLES20.GL_FLOAT_VEC3:
					GLES20.glUniform3fv(mId, mElements, val, 0);
					break;
				case GLES20.GL_FLOAT_VEC4:
					GLES20.glUniform4fv(mId, mElements, val, 0);
					break;
				case GLES20.GL_FLOAT_MAT2:
					GLES20.glUniformMatrix2fv(mId, mElements, false, val, 0);
					break;
				case GLES20.GL_FLOAT_MAT3:
					GLES20.glUniformMatrix3fv(mId, mElements, false, val, 0);
					break;
				case GLES20.GL_FLOAT_MAT4:
					GLES20.glUniformMatrix4fv(mId, mElements, false, val, 0);
					break;
				default:
					Log.e(TAG, "Unsupported parameter type for shader parameter " + mName);
					return false;
			}
			return true;
		}
	}
	
	public static class SamplerInfo extends ParamInfo {
		public int mSamplerIndex;
		
		public SamplerInfo(int id, int type, int elements, String name, int samplerIndex) {
			super(id, type, elements, name);
			mSamplerIndex = samplerIndex;
		}
		
		public boolean apply(GLESTexture texture) {
			if (texture == null)
				return false;
			return texture.apply(GLES20.GL_TEXTURE0 + mSamplerIndex);
		}
	}

	public static class AttribInfo extends ParamInfo {
		public boolean mNormalized;
		
		public AttribInfo(int id, int type, int elements, String name, boolean normalized) {
			super(id, type, elements, name);
			mNormalized = normalized;
		}
	}
	
    public static class ParamArray<T extends ParamInfo> {
    	public ArrayList<T> mParams;
    	
    	public ParamArray() {
    		mParams = new ArrayList<T>();
    	}
    	
    	public void addParamInfo(T param) {
    		mParams.add(param);
    	}
    	
    	public int getParamIndex(String name) {
    		for (int i = 0; i < mParams.size(); i++)
    			if (mParams.get(i).mName.equals(name))
    				return i;
    		return -1;
    	}
    	
    	public T getParam(int i) {
    		return mParams.get(i);
    	}
    	
    	public T getParam(String name) {
    		for (T param: mParams) {
    			if (param.mName.equals(name))
    				return param;
    		}
    		return null;
    	}
    	
    	public int getParamCount(ParamSource source) {
    		if (source == ParamSource.UNKNOWN)
    			return mParams.size();
    		int count = 0;
    		for (T param: mParams)
    			if (param.getSource() == source)
    				count++;
    		return count;
    	}
    	
    	public int getParamCount() {
    		return mParams.size();
    	}
    }
	
    public static class UniformArray extends ParamArray<UniformInfo> {
    }

    public static class SamplerArray extends ParamArray<SamplerInfo> {
    }

    public static class AttribArray extends ParamArray<AttribInfo> {
    	public int mStride;
    	
    	public AttribArray() {
    		super();
    		mStride = 0;
    	}

    	@Override
    	public void addParamInfo(AttribInfo param) {
    		super.addParamInfo(param);
    		mStride += param.getSize();
    	}
    }
    
	protected GLESRenderer mRenderer;
    protected String mName;
	protected int mVertexShader, mFragmentShader;
	protected int mProgram;
	protected UniformArray mUniforms, mPerInstanceUniforms;
	protected SamplerArray mSamplers;
	protected AttribArray mAttribs;
	protected long mLastFrameID;
	
	public GLESShader(GLESRenderer renderer, String name) {
		mRenderer = renderer;
		mVertexShader = 0;
		mFragmentShader = 0;
		mProgram = 0;
		mName = name;
		mLastFrameID = 0;
	}
	
	public boolean init(String vertexSrc, String fragmentSrc) {
		mVertexShader = compileShader(GLES20.GL_VERTEX_SHADER, vertexSrc);
		if (mVertexShader == 0)
			return false;
		mFragmentShader = compileShader(GLES20.GL_FRAGMENT_SHADER, fragmentSrc);
		if (mFragmentShader == 0)
			return false;

		mProgram = GLES20.glCreateProgram();
		if (mProgram == 0)
			return false;
		GLES20.glAttachShader(mProgram, mVertexShader);
		GLES20.glAttachShader(mProgram, mFragmentShader);
		GLES20.glLinkProgram(mProgram);
		int[] result = new int[1];
		GLES20.glGetProgramiv(mProgram, GLES20.GL_LINK_STATUS, result, 0);
		if (result[0] == 0) {
			Log.e(TAG, "Error linking shader program for " + mName + ":");
			Log.e(TAG, GLES20.glGetProgramInfoLog(mProgram));
			GLES20.glDeleteProgram(mProgram);
			mProgram = 0;
		}
		if (!initParams())
			return false;
		return isValid();
	}
	
	public boolean init(Context context, int vertexSrcResource, int fragmentSrcResource) {
		String vertexSrc, fragmentSrc;
		vertexSrc = readString(context.getResources().openRawResource(vertexSrcResource));
		fragmentSrc = readString(context.getResources().openRawResource(fragmentSrcResource));
		return init(vertexSrc, fragmentSrc);
	}
	
	public boolean initParams()	{
		if (!initUniformsAndSamplers())
			return false;
		if (!initAttribs())
			return false;
		return true;
	}
	
	public boolean initUniformsAndSamplers() {
		int i, uniformCount, nameBufLen, type, elements, nameLen, location;
		String name;
		int[] result = new int[1];
		int[] params = new int[3];

		GLES20.glGetProgramiv(mProgram, GLES20.GL_ACTIVE_UNIFORM_MAX_LENGTH, result, 0);
		nameBufLen = result[0] + 1;
		byte[] nameBuf = new byte[nameBufLen];

		GLES20.glGetProgramiv(mProgram, GLES20.GL_ACTIVE_UNIFORMS, result, 0);
		uniformCount = result[0];
		
		mUniforms = new UniformArray();
		mSamplers = new SamplerArray();
		GLES20.glUseProgram(mProgram);
		if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
			Log.e(TAG, "initUniformsAndSamplers() - failed setting program");
			return false;
		}
		for (i = 0; i < uniformCount; i++) {
			GLES20.glGetActiveUniform(mProgram, i, nameBufLen, params, 0, params, 1, params, 2, nameBuf, 0);
			if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
				Log.e(TAG, "initUniformsAndSamplers() - failed getting data for uniform #" + i);
				return false;
			}
			type = params[2];
			elements = params[1];
			nameLen = params[0];
			name = new String(nameBuf, 0, nameLen);
			location = GLES20.glGetUniformLocation(mProgram, name);
			if (type == GLES20.GL_SAMPLER_2D || type == GLES20.GL_SAMPLER_CUBE) {
				int samplerIndex = mSamplers.mParams.size();
				mSamplers.addParamInfo(new SamplerInfo(location, type, elements, name, samplerIndex));
				GLES20.glUniform1i(location, samplerIndex);
			} else 
				mUniforms.addParamInfo(new UniformInfo(location, type, elements, name, this));
		}
		mPerInstanceUniforms = new UniformArray();
		for (UniformInfo uni: mUniforms.mParams) 
			if (uni.mSource == ParamSource.PER_INSTANCE)
				mPerInstanceUniforms.mParams.add(uni);
		return true;
	}
	
	public boolean initAttribs() {
		int i, attribCount, nameBufLen, type, elements, nameLen, location;
		String name;
		int[] result = new int[1];
		int[] params = new int[3];

		GLES20.glGetProgramiv(mProgram, GLES20.GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, result, 0);
		nameBufLen = result[0] + 1;
		byte[] nameBuf = new byte[nameBufLen];

		GLES20.glGetProgramiv(mProgram, GLES20.GL_ACTIVE_ATTRIBUTES, result, 0);
		attribCount = result[0];
		
		mAttribs = new AttribArray();
		for (i = 0; i < attribCount; i++) {
			GLES20.glGetActiveAttrib(mProgram, i, nameBufLen, params, 0, params, 1, params, 2, nameBuf, 0);
			if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
				Log.e(TAG, "initAttribs() - failed getting data for attribute #" + i);
				return false;
			}
			type = params[2];
			elements = params[1];
			nameLen = params[0];
			name = new String(nameBuf, 0, nameLen); 
			location = GLES20.glGetAttribLocation(mProgram, name);
			mAttribs.addParamInfo(new AttribInfo(location, type, elements, name, false));
		}
		return true;
	}
	
	public void done() {
		if (mProgram != 0) {
			GLES20.glDeleteProgram(mProgram);
			mProgram = 0;
		}
		if (mFragmentShader != 0) {
			GLES20.glDeleteShader(mFragmentShader);
			mFragmentShader = 0;
		}
		if (mVertexShader != 0) {
			GLES20.glDeleteShader(mVertexShader);
			mVertexShader = 0;
		}
		mUniforms = null;
		mSamplers = null;
		mAttribs = null;
	}
	
	@Override
	public void finalize() {
		done();
	}
	
	public String getName() { return mName; } 
	
	public String[] getAttribNames() {
		String[] attribNames = new String[mAttribs.mParams.size()];
		for (int i = 0; i < attribNames.length; ++i)
			attribNames[i] = mAttribs.mParams.get(i).mName;
		return attribNames;
	}
	
	public boolean isValid() { return GLES20.glIsProgram(mProgram);	}
	
	public boolean setPerFrame(GLESModel model) {
		if (mLastFrameID >= mRenderer.mFrameID)
			return true;
		mLastFrameID = mRenderer.mFrameID;
		boolean result = true;
		for (UniformInfo uni: mUniforms.mParams) {
			if (uni.mSource == ParamSource.PER_FRAME) {
				result &= uni.mSetter.set(uni, model);
			}
		}
		return result;
	}
	
	public boolean setPerInstance(GLESModel model) {
		boolean result = true;
		for (UniformInfo uni: mPerInstanceUniforms.mParams) 
			result &= uni.mSetter.set(uni, model);
		return result;
	}
	
	public boolean apply() {
		GLES20.glUseProgram(mProgram);
		return GLES20.glGetError() == GLES20.GL_NO_ERROR;
	}
	
	public int compileShader(int shaderType, String shaderSrc) {
		int shader = GLES20.glCreateShader(shaderType);
		if (shader == 0)
			return shader;
		GLES20.glShaderSource(shader, shaderSrc);
		GLES20.glCompileShader(shader);
		int[] result = new int[1];
		GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, result, 0);
		if (result[0] == 0) {
			Log.e(TAG, "Error compiling shader " + shaderType + " for " + mName + ":");
			Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
			GLES20.glDeleteShader(shader);
			shader = 0;
		}
		return shader;
	}
	
	public static int getTypeBase(int type) {
		switch (type) {
			case GLES20.GL_FLOAT_VEC2:
			case GLES20.GL_FLOAT_VEC3:
			case GLES20.GL_FLOAT_VEC4:
			case GLES20.GL_FLOAT_MAT2:
			case GLES20.GL_FLOAT_MAT3:
			case GLES20.GL_FLOAT_MAT4:
				return GLES20.GL_FLOAT;
			case GLES20.GL_UNSIGNED_SHORT_4_4_4_4:
			case GLES20.GL_UNSIGNED_SHORT_5_5_5_1:
			case GLES20.GL_UNSIGNED_SHORT_5_6_5:
				return GLES20.GL_UNSIGNED_SHORT;
			case GLES20.GL_INT_VEC2:
			case GLES20.GL_INT_VEC3:
			case GLES20.GL_INT_VEC4:
				return GLES20.GL_INT;
			default: 
				return type;
		}
	}
	
	public static int getTypeElements(int type) {
		switch (type) {
			case GLES20.GL_FLOAT_VEC2:
			case GLES20.GL_INT_VEC2:
				return 2;
			case GLES20.GL_FLOAT_VEC3:
			case GLES20.GL_INT_VEC3:
				return 3;
			case GLES20.GL_FLOAT_VEC4:
			case GLES20.GL_INT_VEC4:
			case GLES20.GL_FLOAT_MAT2:
				return 4;
			case GLES20.GL_FLOAT_MAT3:
				return 9;
			case GLES20.GL_FLOAT_MAT4:
				return 16;
			default:
				return 1;
		}
	}
	
	public static int getBaseTypeSize(int typeBase) {
		switch (typeBase) {
			case GLES20.GL_FLOAT:
				return Float.SIZE / Byte.SIZE;
			case GLES20.GL_BYTE:
			case GLES20.GL_UNSIGNED_BYTE:
				return 1;
			case GLES20.GL_SHORT:
			case GLES20.GL_UNSIGNED_SHORT:
				return Short.SIZE / Byte.SIZE;
			case GLES20.GL_INT:
			case GLES20.GL_UNSIGNED_INT:
				return Integer.SIZE / Byte.SIZE;
			default:
				Log.e(TAG, "getBaseTypeSize() - unknown parameter type");
				return 0;
		}
	}
	
	public static int getTypeSize(int type, int elements) {
		int bytes, typeBase, typeElements;
		typeBase = getTypeBase(type);
		typeElements = getTypeElements(type);
		bytes = getBaseTypeSize(typeBase) * typeElements * elements;
		return bytes;
	}
	
	public static String readString(InputStream is) {
		if (is == null)
			return "";
		Writer writer = new StringWriter();
		char[] buf = new char[1024];
		Reader reader = new BufferedReader(new InputStreamReader(is));
		try {
			while (true) {
				int i = reader.read(buf);
				if (i <= 0)
					break;
				writer.write(buf, 0, i);
			}
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				is.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		return writer.toString();
	}
}
