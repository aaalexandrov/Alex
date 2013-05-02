package bg.alex_iii.GLES;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.HashMap;

import android.annotation.SuppressLint;
import android.opengl.Matrix;

public class TextHolder {
	public GLESRenderer mRenderer;
	public GLESFont mFont;
	public GLESMaterial mMaterial;
	public GLESModel mModel;
	Alignment mXAlign, mYAlign;
	Allocator mVBAllocator;
	int mNextId;
	HashMap<Integer, Line> mLines;
	
	public enum Alignment {
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,
		CENTER,
		BASELINE,
	}
	
	class Line {
		float mX, mY;
		String mText;
		int mVBOffset, mIBOffset;
		
		int getVBSize() {
			return mText.length() * 4 * getVertexStride();
		}
		
		int getIBSize() {
			return mText.length() * 6 * Short.SIZE / 8;
		}
	}
	
	@SuppressLint("UseSparseArrays")
	public TextHolder(GLESRenderer renderer, GLESFont font, GLESMaterial material, int characters) {
		mRenderer = renderer;
		mFont = font;
		mMaterial = material;
		mNextId = 1;
		mLines = new HashMap<Integer, Line>();
		mXAlign = Alignment.LEFT;
		mYAlign = Alignment.BASELINE;
		initBuffers(characters);
	}
	
	boolean initBuffers(int characters) {
		GLESShader.AttribArray vertexAttrib = GLESBuffer.attribsFromVertex(new GLESUtil.VertexPosUV(0, 0, 0, 0, 0), false, null);
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
		indexBuffer.order(ByteOrder.nativeOrder());
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
	
	public void setAlignment(Alignment xAlign, Alignment yAlign) {
		mXAlign = xAlign;
		mYAlign = yAlign;
	}
	
	public int addLine(float x, float y, String text) {
		x = getXAligned(x, text);
		y = getYAligned(y, text);
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

	float getXAligned(float x, String text) {
		switch (mXAlign) {
			case CENTER:
				x -= text.length() * mFont.mCharWidth / 2;
				break;
			case RIGHT:
				x -= text.length() * mFont.mCharWidth;
				break;
		}
		return x;
	}
	
	float getYAligned(float y, String text) {
		switch (mYAlign) {
			case TOP:
				y += Math.ceil(mFont.mFontMetrics.top); 
				break;
			case BOTTOM:
				y += Math.ceil(mFont.mFontMetrics.bottom);
				break;
			case CENTER:
				y += Math.ceil(-mFont.getCharHeight() / 2 + mFont.mFontMetrics.bottom);
				break;
		}
		return y;
	}
	
	int getVertexStride() {
		return mModel.mGeometry.mVertices.getStride();
	}
	
	void addLineGeometry(Line line) {
		int vbSize = line.getVBSize();
		int vbOffset = mVBAllocator.alloc(vbSize);
		if (vbOffset >= 0) {
			line.mVBOffset = vbOffset;
			GLESUtil.VertexPosUV[] fontVertices = mFont.createVertices(line.mText, line.mX, line.mY);
			ByteBuffer buffer = GLESBuffer.bufferFromData(fontVertices, mModel.mGeometry.mVertices.mFormat);
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
			int vertexStride = getVertexStride();
			doneBuffers();
			initBuffers(newSize / vertexStride);
			for (Line l: mLines.values()) 
				addLineGeometry(l);
		}
	}
	
	void addLineIndices(Line line) {
		line.mIBOffset = mModel.mGeometry.mIndices.getSizeInBytes();
		int baseVertex = line.mVBOffset / getVertexStride();
		short[] indices = mFont.createIndices(line.mText, baseVertex);
		ByteBuffer indexBuffer = ByteBuffer.allocate(indices.length * Short.SIZE / 8);
		indexBuffer.order(ByteOrder.nativeOrder());
		for (short s: indices)
			indexBuffer.putShort(s);
		indexBuffer.rewind();
		mModel.mGeometry.mIndices.update(indexBuffer.capacity(), indexBuffer, line.mIBOffset);
		mModel.mGeometry.mIndices.setSizeInBytes(mModel.mGeometry.mIndices.getSizeInBytes() + indexBuffer.capacity());
	}
}
