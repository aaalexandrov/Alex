package bg.alex_iii.GLES;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;

import android.opengl.Matrix;

public class TextHolder {
	GLESRenderer mRenderer;
	GLESFont mFont;
	GLESMaterial mMaterial;
	Allocator mVBAllocator;
	GLESModel mModel;
	int mNextId;
	HashMap<Integer, Line> mLines;
	
	class Line {
		float mX, mY;
		String mText;
		int mVBOffset, mIBOffset;
		
		int getVBSize() {
			return mText.length() * 4 * mMaterial.mShader.mAttribs.mStride;
		}
		
		int getIBSize() {
			return mText.length() * 6 * Short.SIZE / 8;
		}
	}
	
	public TextHolder(GLESRenderer renderer, GLESFont font, GLESMaterial material, int characters) {
		mRenderer = renderer;
		mFont = font;
		mMaterial = material;
		mNextId = 1;
		mLines = new HashMap<Integer, Line>();
		initBuffers(characters);
	}
	
	boolean initBuffers(int characters) {
		GLESShader.AttribArray vertexAttrib = mMaterial.mShader.mAttribs;
		int vbSize = vertexAttrib.mStride * 4 * characters;
		mVBAllocator = new Allocator(vbSize);

		ByteBuffer buffer = ByteBuffer.allocate(vbSize);
		buffer.order(ByteOrder.nativeOrder());
		GLESBuffer vb = new GLESBuffer(false, GLESBuffer.Usage.DYNAMIC_DRAW);
		if (!vb.init(vertexAttrib, buffer))
			return false;
		vb.setCount(0);
		
		int ibSize = Short.SIZE / 8 * 6 * characters;
		ByteBuffer indexBuffer = ByteBuffer.allocate(ibSize);
		GLESBuffer ib = new GLESBuffer(true, GLESBuffer.Usage.DYNAMIC_DRAW);
		if (!ib.init(null, indexBuffer))
			return false;
		ib.setCount(0);
		
		GLESGeometry geometry = new GLESGeometry(vb, ib, GLESGeometry.PrimitiveType.TRIANGLES);
		
		float[] transform = new float[16];
		Matrix.setIdentityM(transform, 0);
		mModel = new GLESModel(geometry, mMaterial, transform);
		
		return true;
	}
	
	void doneBuffers() {
		if (mModel != null) {
			mModel.mGeometry.done();
			mModel = null;
			mVBAllocator = null;
		}
	}
	
	public void done() {
		doneBuffers();
	}
	
	@Override
	public void finalize() {
		done();
	}
	
	public int addLine(float x, float y, String text) {
		Line line = new Line();
		line.mX = x;
		line.mY = y;
		line.mText = text;
		
		int id = mNextId++;
		mLines.put(id, line);
		addLineGeometry(line);
		
		return id;
	}
	
	public boolean removeLine(int lineId) {
		Line line = mLines.get(lineId);
		if (line == null)
			return false;
		mLines.remove(lineId);
		mVBAllocator.free(line.mVBOffset);
		if (line.mVBOffset + line.getVBSize() >= mModel.mGeometry.mVertices.getSizeInBytes()) {
			int maxOffset = 0;
			for (Line l: mLines.values())
				maxOffset = Math.max(maxOffset, l.mVBOffset + l.getVBSize());
			mModel.mGeometry.mVertices.setSizeInBytes(maxOffset);
		}
		if (line.mIBOffset + line.getIBSize() >= mModel.mGeometry.mIndices.getSizeInBytes())
			mModel.mGeometry.mIndices.setSizeInBytes(line.mIBOffset);
		else {
			mModel.mGeometry.mIndices.setSizeInBytes(0);
			for (Line l: mLines.values()) 
				addLineIndices(l);
		}
		return true;
	}
	
	void addLineGeometry(Line line) {
		int vbSize = line.getVBSize();
		int vbOffset = mVBAllocator.alloc(vbSize);
		if (vbOffset >= 0) {
			line.mVBOffset = vbOffset;
			GLESUtil.VertexPosUV[] fontVertices = mFont.createVertices(line.mText, line.mX, line.mY, mRenderer.mSurfaceWidth, mRenderer.mSurfaceHeight);
			ByteBuffer buffer = GLESBuffer.bufferFromData(fontVertices, mMaterial.mShader.mAttribs);
			assert vbSize == buffer.capacity();
			mModel.mGeometry.mVertices.update(buffer.capacity(), buffer, vbOffset);
			if (vbOffset >= mModel.mGeometry.mVertices.getSizeInBytes())
				mModel.mGeometry.mVertices.setSizeInBytes(mModel.mGeometry.mVertices.getSizeInBytes() + vbSize);
			
			addLineIndices(line);
		} else {
			int newSize;
			if (mVBAllocator.mFree >= vbSize)
				newSize = mVBAllocator.mSize;
			else
				newSize = Math.min(mVBAllocator.mSize * 3 / 2, mVBAllocator.mSize + vbSize);
			doneBuffers();
			initBuffers(newSize / mMaterial.mShader.mAttribs.mStride);
			for (Line l: mLines.values()) 
				addLineGeometry(l);
		}
	}
	
	void addLineIndices(Line line) {
		line.mIBOffset = mModel.mGeometry.mIndices.getSizeInBytes();
		int baseVertex = line.mVBOffset / mMaterial.mShader.mAttribs.mStride;
		short[] indices = mFont.createIndices(line.mText, baseVertex);
		ByteBuffer indexBuffer = ByteBuffer.allocate(indices.length * Short.SIZE / 8);
		for (short s: indices)
			indexBuffer.putShort(s);
		mModel.mGeometry.mIndices.update(indexBuffer.capacity(), indexBuffer, line.mIBOffset);
		mModel.mGeometry.mIndices.setSizeInBytes(mModel.mGeometry.mIndices.getSizeInBytes() + indexBuffer.capacity());
	}
}
