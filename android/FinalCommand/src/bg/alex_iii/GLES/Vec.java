package bg.alex_iii.GLES;

public class Vec {
	public static final float EPS = 0.0001f; 
	
	public interface BinaryOp {
		public float op(float f0, float f1);
	}

	public static final BinaryOp mAddition = new BinaryOp() {
		public float op(float f0, float f1) { return f0 + f1; }
	}; 

	public static final BinaryOp mSubtraction = new BinaryOp() {
		public float op(float f0, float f1) { return f0 - f1; }
	}; 

	public static final BinaryOp mMultiplication = new BinaryOp() {
		public float op(float f0, float f1) { return f0 * f1; }
	}; 

	public static final BinaryOp mDivision = new BinaryOp() {
		public float op(float f0, float f1) { return f0 / f1; }
	}; 
	
	public static float[] componentOp(float[] vec0, float[] vec1, BinaryOp op) {
		int common = Math.min(vec0.length, vec1.length);
		int dim = Math.max(vec0.length, vec1.length);
		float[] result = new float[dim];
		int i;
		for (i = 0; i < common; i++) 
			result[i] = op.op(vec0[i], vec1[i]);
		float[] rest = null;
		if (vec0.length > common)
			rest = vec0;
		else
			if (vec1.length > common)
				rest = vec1;
		if (rest != null)
			while (i < dim) {
				result[i] = rest[i];
				i++;
			}
		return result;
	}
	
	public static float[] componentOp(float[] vec, float f, BinaryOp op) {
		float[] result = new float[vec.length];
		for (int i = 0; i < vec.length; i++) 
			result[i] = op.op(vec[i], f);
		return result;
	}
	
	public static float[] componentOp(float f, float[] vec, BinaryOp op) {
		float[] result = new float[vec.length];
		for (int i = 0; i < vec.length; i++) 
			result[i] = op.op(f, vec[i]);
		return result;
	}
	
	public static float[] get(float x, float y, float z, float w) {
		float[] result = { x, y, z, w };
		return result;
	}

	public static float[] get(float x, float y, float z) {
		float[] result = { x, y, z };
		return result;
	}

	public static float[] get(float x, float y) {
		float[] result = { x, y };
		return result;
	}
	
	public static float[] set(float[] vec, float x, float y, float z, float w) {
		vec[0] = x;
		vec[1] = y;
		vec[2] = z;
		vec[3] = w;
		return vec;
	}

	public static float[] set(float[] vec, float x, float y, float z) {
		vec[0] = x;
		vec[1] = y;
		vec[2] = z;
		return vec;
	}

	public static float[] set(float[] vec, float x, float y) {
		vec[0] = x;
		vec[1] = y;
		return vec;
	}
	
	public static float[] getPolar(float r, float theta, float phi) {
		float[] result = new float[3];
		float sinTheta = (float) Math.sin(theta);
		result[0] = r * sinTheta * (float) Math.cos(phi); 
		result[1] = r * sinTheta * (float) Math.sin(phi);
		result[2] = r * (float) Math.cos(theta); 
		return result;
	}
	
	public static float[] toPolar(float[] vec) {
		assert(vec.length == 3);
		float r, theta, phi;
		r = getLength(vec);
		theta = (float) Math.acos(vec[2] / r);
		float sinTheta = (float) Math.sin(theta);
		phi = (float) Math.atan2(vec[1] / sinTheta, vec[0] / sinTheta);
		float[] result = get(r, theta, phi);
		return result;
	}
	
	public static float[] getZero(int dim) {
		float[] result = new float[dim];
		for (int i = 0; i < dim; ++i)
			result[i] = 0;
		return result;
	}
	
	public static float[] getOne(int dim, int oneIndex) {
		float[] result = new float[dim];
		for (int i = 0; i < dim; ++i)
			result[i] = oneIndex < 0 || i == oneIndex ? 1 : 0;
		return result;
	}
	
	public static int getMaxComponentIndex(float[] vec) {
		int maxComponent = 0;
		for (int i = 1; i < vec.length; ++i)
			if (Math.abs(vec[i]) > Math.abs(vec[maxComponent]))
				maxComponent = i;
		return maxComponent;
	}
	
	public static float[] getOrthoNormal(float[] vec) {
		int maxComponent = getMaxComponentIndex(vec);
		float[] result = getOne(vec.length, (maxComponent + 1) % vec.length);
		result = sub(result, mul(dot(result, vec), vec));
		normalize(result);
		assert(Math.abs(dot(vec, result)) < EPS);
		return result;
	}
	
	public static float[][] getOrthoNormals(float[] vec) {
		int maxComponent = getMaxComponentIndex(vec);
		float[][] results = new float[vec.length][];
		results[0] = getNormalized(vec);
		for (int i = 1; i < vec.length; ++i) {
			results[i] = getOne(vec.length, (maxComponent + i) % vec.length);
			for (int j = 0; j < i; ++j) 
				results[i] = sub(results[i], mul(dot(results[i], results[j]), results[j]));
			normalize(results[i]);
		}
		return results;
	}
	
	public static boolean IsZero(float[] vec) {
		for (int i = 0; i < vec.length; ++i)
			if (Math.abs(vec[i]) >= EPS) 
				return false;
		return true;
	}

	public static boolean IsEqual(float[] vec0, float[] vec1) {
		if (vec0.length != vec1.length)
			return false;
		for (int i = 0; i < vec0.length; ++i)
			if (Math.abs(vec0[i] - vec1[i]) >= EPS) 
				return false;
		return true;
	}
	
	public static float getLengthSquare(float[] vec) {
		float len2 = 0;
		for (int i = 0; i < vec.length; i++)
			len2 += vec[i] * vec[i];
		return len2;
	}
	
	public static float getLength(float[] vec) {
		return (float) Math.sqrt(getLengthSquare(vec));
	}
	
	public static float[] normalize(float[] vec) {
		float len = getLength(vec);
		if (len > 0)
			for (int i = 0; i < vec.length; i++)
				vec[i] /= len;
		return vec;
	}
	
	public static float[] getNormalized(float[] vec) {
		float len = getLength(vec);
		float[] result = new float[vec.length];
		if (len > 0)
			for (int i = 0; i < vec.length; i++)
				result[i] = vec[i] / len;
		else
			for (int i = 0; i < vec.length; i++)
				result[i] = 0;
		return result;
	}
	
	public static float[] scale(float length, float[] vec) {
		float len = getLength(vec);
		if (len > 0) {
			float factor = length / len;
			for (int i = 0; i < vec.length; i++)
				vec[i] *= factor;
		}
		return vec;
	}

	public static float[] getScaled(float length, float[] vec) {
		float len = getLength(vec);
		if (len > 0) {
			float factor = length / len;
			return mul(factor, vec); 
		}
		return getZero(vec.length);
	}
	
	public static float dot(float[] vec0, float[] vec1) {
		int dim = Math.min(vec0.length, vec1.length);
		float result = 0;
		for (int i = 0; i < dim; i++)
			result += vec0[i] * vec1[i];
		return result;
	}
	
	public static float[] cross(float[] vec0, float[] vec1) {
		assert(vec0.length == 3 && vec1.length == 3);
		float[] result = new float[3];
		result[0] = vec0[1] * vec1[2] - vec0[2] * vec1[1];
		result[1] = vec0[2] * vec1[0] - vec0[0] * vec1[2];
		result[2] = vec0[0] * vec1[1] - vec0[1] * vec1[0];
		return result;
	}
	
	public static float[] neg(float[] vec) {
		return sub(0, vec);
	}
	
	public static float[] add(float[] vec0, float[] vec1) {
		return componentOp(vec0, vec1, mAddition);
	}

	public static float[] add(float[] vec, float f) {
		return componentOp(vec, f, mAddition);
	}

	public static float[] add(float f, float[] vec) {
		return componentOp(f, vec, mAddition);
	}
	
	public static float[] sub(float[] vec0, float[] vec1) {
		return componentOp(vec0, vec1, mSubtraction);
	}

	public static float[] sub(float[] vec, float f) {
		return componentOp(vec, f, mSubtraction);
	}
	
	public static float[] sub(float f, float[] vec) {
		return componentOp(f, vec, mSubtraction);
	}
	
	public static float[] mul(float[] vec0, float[] vec1) {
		return componentOp(vec0, vec1, mMultiplication);
	}

	public static float[] mul(float[] vec, float f) {
		return componentOp(vec, f, mMultiplication);
	}

	public static float[] mul(float f, float[] vec) {
		return componentOp(f, vec, mMultiplication);
	}
	
	public static float[] div(float[] vec0, float[] vec1) {
		return componentOp(vec0, vec1, mDivision);
	}

	public static float[] div(float[] vec, float f) {
		return componentOp(vec, f, mDivision);
	}

	public static float[] div(float f, float[] vec) {
		return componentOp(f, vec, mDivision);
	}
}
