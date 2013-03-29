package bg.alex_iii.GLES;

import android.opengl.Matrix;

public class GLESModel {
	public static final String TAG = "GLESModel";
	
	public GLESGeometry mGeometry;
	public GLESMaterial mMaterial;
	public float[] mTransform;
	
	public static <V,I> GLESModel create(V vertices, I indices, GLESGeometry.PrimitiveType primitiveType, GLESMaterial material) {
		if (material == null)
			return null;
		GLESBuffer vb, ib;
		vb = new GLESBuffer(false, GLESBuffer.Usage.STATIC_DRAW);
		if (!vb.init(vertices))
			return null;
		ib = new GLESBuffer(true, GLESBuffer.Usage.STATIC_DRAW);
		if (!ib.init(indices))
			return null;
		GLESGeometry geometry = new GLESGeometry(vb, ib, primitiveType);
		float[] transform = new float[16];
		Matrix.setIdentityM(transform, 0);
		GLESModel model = new GLESModel(geometry, material, transform);
		return model;
	}
	
	public GLESModel(GLESGeometry geometry, GLESMaterial material, float[] transform) {
		mGeometry = geometry;
		mMaterial = material;
		mTransform = transform;
	}

	public boolean apply() {
		if (!mMaterial.apply(this))
			return false;
		if (!mGeometry.apply(mMaterial.mShader))
			return false;
		return true;
	}
	
	public boolean render() {
		if (!apply())
			return false;
		if (!mGeometry.render())
			return false;
		return true;
	}
}
