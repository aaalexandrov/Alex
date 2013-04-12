package bg.alex_iii.FinalCommand;

import java.util.ArrayList;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESBuffer;
import bg.alex_iii.GLES.GLESGeometry;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESRenderer;
import bg.alex_iii.GLES.GLESUtil.VertexPos;

public class LineHolder {
	GLESRenderer mRenderer;
	ArrayList<VertexPos> mVertices;
	GLESModel mModel;
	
	LineHolder(GLESRenderer renderer) {
		mRenderer = renderer;
		mVertices = new ArrayList<VertexPos>();
		float[] transform = new float[16];
		Matrix.setIdentityM(transform, 0);
		mModel = new GLESModel(null, mRenderer.mMaterials.get("color"), transform);
	}
	
	void clearPoints() {
		mVertices.clear();
	}
	
	int addPoint(float x, float y, float z) {
		mVertices.add(new VertexPos(x, y, z));
		return mVertices.size() - 1;
	}
	
	boolean updateModel() {
		if (mModel.mGeometry != null) {
			// Explicitly release resources because we don't know when the buffers will get garbage collected
			mModel.mGeometry.done();
			mModel.mGeometry = null;
		}
		
		if (mVertices.size() == 0) 
			return true;
		
		short[] indices = new short[mVertices.size()];
		for (int i = 0; i < mVertices.size(); ++i)
			indices[i] = (short) i;
		GLESBuffer vb, ib;
		vb = new GLESBuffer(false, GLESBuffer.Usage.STREAM_DRAW);
		VertexPos[] vbArray = new VertexPos[mVertices.size()];
		if (!vb.init(mVertices.toArray(vbArray), null))
			return false;
		ib = new GLESBuffer(true, GLESBuffer.Usage.STREAM_DRAW);
		if (!ib.init(indices, null))
			return false;
		mModel.mGeometry = new GLESGeometry(vb, ib, GLESGeometry.PrimitiveType.LINES);
		return true;
	}
	
	boolean render() {
		if (mModel.mGeometry == null)
			return true;
		
		return mModel.render();
	}
}
