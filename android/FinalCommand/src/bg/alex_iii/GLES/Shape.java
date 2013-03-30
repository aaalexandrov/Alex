package bg.alex_iii.GLES;

public class Shape {
	public static boolean isPointInsideSphere(float[] point, float[] center, float radius) {
		return Vec.getLengthSquare(Vec.sub(point, center)) <= radius * radius;
	}
	
	public static boolean isPointInsideCylinder(float[] point, float[] origin, float height, float radius) {
		float[] p = Vec.sub(point, origin);
		if (p[2] < 0 || p[2] > height)
			return false;
		if (p[0] * p[0] + p[1] * p[1] > radius * radius)
			return false;
		return true;
	}
	
	public static boolean isPointInsideCone(float[] point, float[] origin, float height, float radius) {
		float[] p = Vec.sub(point, origin);
		if (p[2] < 0 || p[2] > height)
			return false;
		radius = radius * (height - p[2]) / height;
		if (p[0] * p[0] + p[1] * p[1] > radius * radius)
			return false;
		return true;
	}
	
	public static boolean testRayBoxPlaneIntersection(float[] boxMin, float[] boxMax, float[] rayOrigin, float[] rayDirection, float planeValue, int constIndex, float[] factorMinMax) {
		if (Util.isZero(rayDirection[constIndex]))
			if (rayOrigin[constIndex] < boxMin[constIndex] || rayOrigin[constIndex] > boxMax[constIndex])
				return false;
		float factor = (planeValue - rayOrigin[constIndex]) / rayDirection[constIndex];
		float[] p = Vec.add(rayOrigin, Vec.mul(factor, rayDirection));
		int i = (constIndex + 1) % 3;
		if (p[i] < boxMin[i] || p[i] > boxMax[i])
			return true;
		i = (constIndex + 2) % 3;
		if (p[i] < boxMin[i] || p[i] > boxMax[i])
			return true;
		if (factorMinMax[0] > factor)
			factorMinMax[0] = factor;
		if (factorMinMax[1] < factor)
			factorMinMax[1] = factor;
		return true;
	}
	
	public static boolean isRayIntersectingBox(float[] boxMin, float[] boxMax, float[] rayOrigin, float[] rayDirection, float[] factorMinMax) {
		factorMinMax[0] = Float.POSITIVE_INFINITY;
		factorMinMax[1] = Float.NEGATIVE_INFINITY;
		for (int i = 0; i < 3; ++i) {
			if (!testRayBoxPlaneIntersection(boxMin, boxMax, rayOrigin, rayDirection, boxMin[i], i, factorMinMax))
				return false;
			if (!testRayBoxPlaneIntersection(boxMin, boxMax, rayOrigin, rayDirection, boxMax[i], i, factorMinMax))
				return false;
		}
		if (Float.isInfinite(factorMinMax[0]))
			return false;
		return true;
	}
}
