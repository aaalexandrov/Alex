package bg.alex_iii.GLES;

import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import android.opengl.GLES20;
import android.util.Log;

public class GLESBuffer {
	public static final String TAG = "GLESBuffer";

	public enum Usage {
		STREAM_DRAW, STATIC_DRAW, DYNAMIC_DRAW,
	}
	
	protected int mBuffer;
	protected int mType;
	protected int mUsage;
	protected int mSize;
	protected GLESShader.AttribArray mFormat, mLastShaderFormat;
	
	public GLESBuffer(boolean indexBuffer, Usage usage) {
		mBuffer = 0;
		if (indexBuffer) 
			mType = GLES20.GL_ELEMENT_ARRAY_BUFFER;
		else
			mType = GLES20.GL_ARRAY_BUFFER;
		mUsage = usage2GL(usage);
	}
	
	public boolean initBuffer() {
		assert(mBuffer == 0);
		int[] buf = new int[1];
		GLES20.glGenBuffers(1, buf, 0);
		if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
			Log.e(TAG, "Vertex buffer creation failed");
			return false;
		}
		mBuffer = buf[0];
		return true;
	}
	
	public void doneBuffer() {
		assert(mBuffer != 0);
		int[] buf = { mBuffer };
		GLES20.glDeleteBuffers(1, buf, 0);
		mBuffer = 0;
	}
	
	public boolean init(GLESShader.AttribArray format, ByteBuffer bufferData) {
		if (format == null && isIndexBuffer()) {
			format = new GLESShader.AttribArray();
			format.addParamInfo(new GLESShader.AttribInfo(0, GLES20.GL_UNSIGNED_SHORT, 1, "@array", false));
		}
		mFormat = format;
		if (!initBuffer())
			return false;
		mSize = bufferData.capacity();
		assert(mSize % mFormat.mStride == 0);
		GLES20.glBindBuffer(mType, mBuffer);
		GLES20.glBufferData(mType, mSize, bufferData, mUsage);
		return true;
	}

	public <T> boolean init(T dataArray) {
		GLESShader.AttribArray format = attribsFromVertex(Array.get(dataArray, 0), isIndexBuffer());
		ByteBuffer buffer = bufferFromData(dataArray, format);
		if (buffer == null)
			return false;
		return init(format, buffer);
	}
	
	public void done() {
		if (mBuffer != 0) 
			doneBuffer();
	}
	
	@Override
	public void finalize() {
		done();
	}
	
	public boolean isValid() {
		return GLES20.glIsBuffer(mBuffer);
	}

	public boolean isIndexBuffer() {
		return mType == GLES20.GL_ELEMENT_ARRAY_BUFFER;
	}
	
	public int getType() { return mType; }
	public int getUsage() { return mUsage; }
	public int getByteSize() { return mSize; }
	public int getCount() { return mSize / mFormat.mStride; }  
	
	public boolean update(int size, ByteBuffer bufferData, int offset) {
		GLES20.glBindBuffer(mType, mBuffer);
		GLES20.glBufferSubData(mType, offset, size, bufferData);
		if (GLES20.glGetError() != GLES20.GL_NO_ERROR) {
			Log.e(TAG, "Error updating buffer contents");
			return false;
		}
		return true;
	}

	public <T> boolean update(T dataArray) {
		ByteBuffer buffer = bufferFromData(dataArray, mFormat);
		return update(buffer.capacity(), buffer, 0);
	}
	
	public void setShaderFormat(GLESShader.AttribArray shaderFormat) {
		if (mLastShaderFormat == shaderFormat) 
			return;
		for (GLESShader.AttribInfo attrib: mFormat.mParams) {
			GLESShader.AttribInfo shaderAttrib = shaderFormat.getParam(attrib.mName);
			if (shaderAttrib != null)
				attrib.mId = shaderAttrib.mId;
			else
				attrib.mId = -1;
		}
		mLastShaderFormat = shaderFormat;
	}
	
	public boolean apply(GLESShader.AttribArray shaderFormat) {
		GLES20.glBindBuffer(mType, mBuffer);
		boolean result = true;
		if (mType == GLES20.GL_ARRAY_BUFFER) { // vertex buffer
			int offset;
			offset = 0;
			setShaderFormat(shaderFormat);
			for (GLESShader.AttribInfo attrib: mFormat.mParams) {
				if (attrib.mId >= 0) {
					GLES20.glVertexAttribPointer(attrib.mId, attrib.mElements, attrib.mType, attrib.mNormalized, 
							mFormat.mStride, offset);
					if (GLES20.glGetError() != GLES20.GL_NO_ERROR) { 
						Log.e(TAG, "apply() - glVertexAttribPointer failed on attribute " + attrib.mName);
						result = false;
					}
				}
				GLES20.glEnableVertexAttribArray(attrib.mId);
				offset += attrib.getSize();
			}
		}  
		return result;
	}
	
	public static int getUnsignedType(int type) {
		switch (type) {
			case GLES20.GL_SHORT:
				return GLES20.GL_UNSIGNED_SHORT;
			case GLES20.GL_INT:
				return GLES20.GL_UNSIGNED_INT;
			case GLES20.GL_BYTE:
				return GLES20.GL_UNSIGNED_BYTE;
			default:
				return type;
		}
	}
	
	public static void addVertexAttrib(GLESShader.AttribArray attribs, Object vertexElement, int elementId, String elementName, boolean unsigned) {
		int type, elements;
		Class<?> elemClass;
		if (vertexElement.getClass().isArray()) {
			elemClass = vertexElement.getClass().getComponentType();
			elements = Array.getLength(vertexElement); 
			if (elements <= 0)
				return;
		} else {
			elemClass = vertexElement.getClass();
			elements = 1;
		}
		if (elemClass.equals(Float.TYPE) || elemClass.equals(Float.class))
			type = GLES20.GL_FLOAT;
		else if (elemClass.equals(Short.TYPE) || elemClass.equals(Short.class))
			type = GLES20.GL_SHORT;
		else if (elemClass.equals(Integer.TYPE) || elemClass.equals(Integer.class))
			type = GLES20.GL_INT;
		else if (elemClass.equals(Character.TYPE) || elemClass.equals(Character.class))
			type = GLES20.GL_BYTE;
		else if (elemClass.equals(Byte.TYPE) || elemClass.equals(Byte.class))
			type = GLES20.GL_UNSIGNED_BYTE;
		else
			return;
		if (unsigned)
			type = getUnsignedType(type);
		attribs.addParamInfo(new GLESShader.AttribInfo(elementId, type, elements, elementName, false));
	}
	
	public static <T> GLESShader.AttribArray attribsFromVertex(T vertex, boolean indexFormat) {
		GLESShader.AttribArray attribs = new GLESShader.AttribArray();
		Class<?> vertClass = vertex.getClass();
		if (vertClass.equals(Float.class) || vertClass.equals(Short.class) || vertClass.equals(Integer.class)) {
			addVertexAttrib(attribs, vertex, 0, "@" + vertClass.getSimpleName(), indexFormat);
		} else {
			Field[] vertexFields = vertClass.getFields();
			for (int i = 0; i < vertexFields.length; i++) {
				try {
					addVertexAttrib(attribs, vertexFields[i].get(vertex), -1, vertexFields[i].getName(), indexFormat);
				} catch (IllegalAccessException e) {
					Log.e(TAG, "attribsFromVertex(): Error getting array size of field " + vertex.getClass().getName() + "." + vertexFields[i].getName());
					continue;
				}
			}
		}
		if (attribs.getParamCount() <= 0)
			attribs = null;
		return attribs;
	}

	public static <T> void setBufferField(ByteBuffer buffer, T field, int glType) {
		switch (GLESShader.getTypeBase(glType)) {
			case GLES20.GL_FLOAT:
				buffer.putFloat((Float) field);
				break;
			case GLES20.GL_INT:
			case GLES20.GL_UNSIGNED_INT:
				buffer.putInt((Integer) field);
				break;
			case GLES20.GL_BYTE:
			case GLES20.GL_UNSIGNED_BYTE:
				buffer.put((Byte) field);
				break;
			case GLES20.GL_SHORT:
			case GLES20.GL_UNSIGNED_SHORT:				
				buffer.putShort((Short) field);
				break;
			default:
				Log.e(TAG, "setField(): invalid type");
				break;
		}
	}
	
	public static <T> ByteBuffer bufferFromData(T dataArray, GLESShader.AttribArray format) {
		if (format == null)
			return null;
		int elem, elements = Array.getLength(dataArray);
		int i;
		ByteBuffer buffer = ByteBuffer.allocate(format.mStride * elements);
		buffer.order(ByteOrder.nativeOrder());
		for (elem = 0; elem < elements; elem++) {
			for (i = 0; i < format.getParamCount(); i++) {
				GLESShader.AttribInfo param = format.getParam(i);
				Object vertex = Array.get(dataArray, elem);
				try {
					if (param.mName.charAt(0) != '@') {
						Field field = dataArray.getClass().getComponentType().getField(param.mName);
						if (field != null && field.getType().isArray()) {
							int fieldIndex, fieldCount;
							Object fieldObj = field.get(vertex);
							fieldCount = Math.min(Array.getLength(fieldObj), param.mElements * GLESShader.getTypeElements(param.mType));
							for (fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
								setBufferField(buffer, Array.get(fieldObj, fieldIndex), param.mType);
						} else 
							setBufferField(buffer, field.get(vertex), param.mType);
					} else
						setBufferField(buffer, vertex, param.mType);
				} catch (NoSuchFieldException e) {
					Log.e(TAG, "bufferFromData(): Field " + param.mName + " missing in data");
					return null;
				} catch (IllegalAccessException e) {
					Log.e(TAG, "bufferFromData(): Error getting value of field " + param.mName);
					return null;
				}
			}
		}
		buffer.position(0);
		
/*		FloatBuffer fb = buffer.asFloatBuffer();
		float[] f = new float[fb.capacity()];
		fb.get(f);
		if (f.length == 20) {
			for (int i1 = 0; i1 < f.length; i1++)
				Log.e(TAG, Float.toString(f[i1]));
		}

		ShortBuffer sb = buffer.asShortBuffer();
		short[] s = new short[sb.capacity()];
		sb.get(s);
		if (s.length == 6) {
			for (int i1 = 0; i1 < s.length; i1++)
				Log.e(TAG, Short.toString(s[i1]));
		}
*/		
		
		return buffer; 
	}
	
	protected int usage2GL(Usage usage) {
		switch (usage) {
			case STREAM_DRAW:
				return GLES20.GL_STREAM_DRAW;
			case STATIC_DRAW:
				return GLES20.GL_STATIC_DRAW;
			case DYNAMIC_DRAW:
			default:
				return GLES20.GL_DYNAMIC_DRAW;
		}
	}
}
