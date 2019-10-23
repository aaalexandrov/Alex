package bg.alex_iii.GLES;

import java.lang.Math;
import java.util.ArrayList;

import bg.alex_iii.GLES.Vec;

public class GLESUtil {
	
	public static class VertexPos {
		public float[] aPosition;

		public VertexPos(float x, float y, float z) {
			aPosition = new float[3];
			aPosition[0] = x;
			aPosition[1] = y;
			aPosition[2] = z;
		}
	}

	public static class VertexPosColor {
		public float[] aPosition;
		@Normalized
		public byte[] aColor;
		
		public VertexPosColor(float x, float y, float z, byte r, byte g, byte b, byte a) {
			aPosition = Vec.get(x, y, z);
			byte[] rgba = { r, g, b, a };
			aColor = rgba;
		}
		
		public VertexPosColor(float[] pos, byte[] color) {
			assert pos.length == 3 && color.length == 4;
			aPosition = pos;
			aColor = color;
		}
	}
	
	public static class VertexPosNorm {
		public float[] aPosition;
		public float[] aNormal;

		public VertexPosNorm(float x, float y, float z, 
							   float nx, float ny,	float nz) {
			aPosition = new float[3];
			aPosition[0] = x;
			aPosition[1] = y;
			aPosition[2] = z;
			aNormal = new float[3];
			aNormal[0] = nx;
			aNormal[1] = ny;
			aNormal[2] = nz;
		}
		
		public VertexPosNorm(float[] pos, float[] norm) {
			assert pos.length == 3 && norm.length == 3;
			aPosition = pos;
			aNormal = norm;
		}
		
		@Override
		public boolean equals(Object o) {
			if (!(o instanceof VertexPosNorm))
				return false;
			VertexPosNorm vpn = (VertexPosNorm) o;
			return Vec.IsEqual(aPosition, vpn.aPosition) && Vec.IsEqual(aNormal, vpn.aNormal);
		}
	}

	public static class VertexPosUV {
		public float[] aPosition;
		public float[] aTextureCoord;

		public VertexPosUV(float x, float y, float z, 
						   float u, float v) {
			aPosition = new float[3];
			aPosition[0] = x;
			aPosition[1] = y;
			aPosition[2] = z;
			aTextureCoord = new float[2];
			aTextureCoord[0] = u;
			aTextureCoord[1] = v;
		}
	}
	
	public static class VertexPosNormUV {
		public float[] aPosition;
		public float[] aNormal;
		public float[] aTextureCoord;

		public VertexPosNormUV(float x, float y, float z, 
							   float nx, float ny,	float nz, 
							   float u, float v) {
			aPosition = new float[3];
			aPosition[0] = x;
			aPosition[1] = y;
			aPosition[2] = z;
			aNormal = new float[3];
			aNormal[0] = nx;
			aNormal[1] = ny;
			aNormal[2] = nz;
			aTextureCoord = new float[2];
			aTextureCoord[0] = u;
			aTextureCoord[1] = v;
		}
	}

	public static class ShapeDefinition {
		public ArrayList<VertexPosNorm> mVertices = new ArrayList<VertexPosNorm>();
		public ArrayList<Short> mIndices = new ArrayList<Short>();
		
		public int addVertex(VertexPosNorm vertex, boolean mergeWithEqual) {
			if (mergeWithEqual) {
				int vertIndex = mVertices.indexOf(vertex);
				if (vertIndex >= 0)
					return vertIndex;
			}
			mVertices.add(vertex);
			return mVertices.size() - 1;
		}
		
		public void addIndex(int index) {
			mIndices.add((short) index);
		}

		public void addIndices(int i0, int i1, int i2) {
			mIndices.add((short) i0);
			mIndices.add((short) i1);
			mIndices.add((short) i2);
		}
		
		public void addCap(float radius, int sides, float[] center, float[] normal) {
			float[] x = Vec.mul(radius, Vec.getOrthoNormal(normal));
			float[] y = Vec.cross(normal, x);
			
			float angleDelta = (float) (2 * Math.PI) / sides;
			for (int i = 0; i < sides; ++i) {
				float[] pos = Vec.add(Vec.mul((float) Math.cos(i * angleDelta), x), Vec.mul((float) Math.sin(i * angleDelta), y)); 
				pos = Vec.add(pos, center);
				mVertices.add(new VertexPosNorm(pos, normal));
			}
			for (int i = 1; i < sides - 1; ++i) {
				addIndices(0, i, i + 1);
			}
		}
		
		public void addTriangle(float[] p0, float[] p1, float[] p2) {
			float[] dx = Vec.sub(p1, p0);
			float[] dy = Vec.sub(p1, p2);
			float[] normal = Vec.cross(dx, dy);
			Vec.normalize(normal);
			
			int vert0, vert1, vert2;
			vert0 = addVertex(new VertexPosNorm(p0, normal), false);
			vert1 = addVertex(new VertexPosNorm(p1, normal), false);
			vert2 = addVertex(new VertexPosNorm(p2, normal), false);
			
			addIndices(vert1, vert0, vert2);
		}

		public void addTriangle(float[] p0, float[] p1, float[] p2, float[] n0, float[] n1, float[] n2) {
			int vert0, vert1, vert2;
			vert0 = addVertex(new VertexPosNorm(p0, n0), true);
			vert1 = addVertex(new VertexPosNorm(p1, n1), true);
			vert2 = addVertex(new VertexPosNorm(p2, n2), true);
			
			addIndices(vert1, vert0, vert2);
		}
		
		public void addQuad(float[] p00, float[] p01, float[] p10, float[] p11) {
			float[] dx = Vec.sub(p11, p00);
			float[] dy = Vec.sub(p01, p10);
			float[] normal = Vec.cross(dx, dy);
			Vec.normalize(normal);
			
			int vert0, vert1, vert2, vert3;
			vert0 = addVertex(new VertexPosNorm(p00, normal), false);
			vert1 = addVertex(new VertexPosNorm(p01, normal), false);
			vert2 = addVertex(new VertexPosNorm(p10, normal), false);
			vert3 = addVertex(new VertexPosNorm(p11, normal), false);
			
			addIndices(vert0, vert2, vert1);
			addIndices(vert1, vert2, vert3);
		}

		public void addQuad(float[] p00, float[] p01, float[] p10, float[] p11, float[] n00, float[] n01, float[] n10, float[] n11) {
			int vert0, vert1, vert2, vert3;
			vert0 = addVertex(new VertexPosNorm(p00, n00), true);
			vert1 = addVertex(new VertexPosNorm(p01, n01), true);
			vert2 = addVertex(new VertexPosNorm(p10, n10), true);
			vert3 = addVertex(new VertexPosNorm(p11, n11), true);
			
			addIndices(vert0, vert2, vert1);
			addIndices(vert1, vert2, vert3);
		}
		
		public GLESModel createModel(GLESMaterial material) {
			VertexPosNorm[] vertices = new VertexPosNorm[mVertices.size()];
			mVertices.toArray(vertices);
			return GLESModel.create(vertices, mIndices.toArray(), GLESGeometry.PrimitiveType.TRIANGLES, material);
		}
	}
	
	public static ShapeDefinition createPyramid(float radius, int sides, float[] height, boolean smooth) {
		ShapeDefinition shape = new ShapeDefinition();
		
		float[] normal = Vec.get(0, 0, -1);
		shape.addCap(radius, sides, Vec.getZero(3), normal);
		
		VertexPosNorm vPrev = shape.mVertices.get(sides - 1);
		for (int i = 0; i < sides; ++i) {
			VertexPosNorm v = shape.mVertices.get(i);
			if (smooth) {
				float[] prevNorm;
				float[] axis = Vec.sub(height, vPrev.aPosition);
				Vec.normalize(axis);
				prevNorm = Vec.sub(vPrev.aPosition, Vec.mul(Vec.dot(vPrev.aPosition, axis), axis));
				Vec.normalize(Vec.neg(prevNorm));
				axis = Vec.sub(height, v.aPosition);
				Vec.normalize(axis);
				normal =  Vec.sub(v.aPosition, Vec.mul(Vec.dot(v.aPosition, axis), axis));
				Vec.normalize(Vec.neg(normal));
				shape.addTriangle(vPrev.aPosition, v.aPosition, height, prevNorm, normal, Vec.getNormalized(height));
			} else
				shape.addTriangle(vPrev.aPosition, v.aPosition, height);
			vPrev = v;
		}
		
		return shape;
	}
	
	public static ShapeDefinition createPrism(float radius, int sides, float[] height, boolean smooth) {
		ShapeDefinition shape = new ShapeDefinition();
		
		float[] normal = Vec.get(0, 0, -1);
		shape.addCap(radius, sides, Vec.getZero(3), normal);
		normal = Vec.get(0, 0, 1);
		for (int i = 0; i < sides; ++i) {
			float[] pos = shape.mVertices.get(i).aPosition;
			pos = Vec.add(pos, height);
			shape.addVertex(new VertexPosNorm(pos, normal), false);
		}

		int indices = shape.mIndices.size();
		for (int i = 0; i < indices; i += 3)
			shape.addIndices((short) (shape.mIndices.get(i) + sides), (short) (shape.mIndices.get(i + 2) + sides), (short) (shape.mIndices.get(i + 1) + sides));
		
		int prevSide = sides - 1; 
		for (int i = 0; i < sides; ++i) {
			float[] prevPos = shape.mVertices.get(prevSide).aPosition;
			float[] pos = shape.mVertices.get(i).aPosition;
			float[] prevTop = shape.mVertices.get(prevSide + sides).aPosition;
			float[] top = shape.mVertices.get(i + sides).aPosition;
			
			if (smooth) {
				float[] prevNorm, norm;
				prevNorm = Vec.getNormalized(prevPos);
				norm = Vec.getNormalized(pos);
				shape.addQuad(prevPos, pos, prevTop, top, prevNorm, norm, prevNorm, norm);
			} else
				shape.addQuad(prevPos, pos, prevTop, top);
			
			prevSide = i;
		}
		
		return shape;
	}

	public static ShapeDefinition createGlobe(float radius, int parallels, int meridians, boolean smooth) {
		ShapeDefinition shape = new ShapeDefinition();
		
		float meridianAngle = (float) Math.PI * 2 / meridians;
		float parallelAngle = (float) Math.PI / (parallels + 1);
		for (int m = 0; m < meridians; ++m) {
			float[] p00, p01, p10, p11;
			float[] n00, n01, n10, n11;
			p00 = Vec.getPolar(radius, parallelAngle, m * meridianAngle);
			p01 = Vec.getPolar(radius, parallelAngle, (m + 1) * meridianAngle);
			p11 = Vec.get(0, 0, radius);
			if (smooth) {
				n00 = Vec.getNormalized(p00);
				n01 = Vec.getNormalized(p01);
				n11 = Vec.getNormalized(p11);
				shape.addTriangle(p00, p11, p01, n00, n11, n01);
			} else
				shape.addTriangle(p00, p11, p01);
			for (int p = 1; p < parallels; ++p) {
				p00 = Vec.getPolar(radius, p * parallelAngle, m * meridianAngle);
				p01 = Vec.getPolar(radius, p * parallelAngle, (m + 1) * meridianAngle);
				p10 = Vec.getPolar(radius, (p + 1) * parallelAngle, m * meridianAngle);
				p11 = Vec.getPolar(radius, (p + 1) * parallelAngle, (m + 1) * meridianAngle);
				if (smooth) {
					n00 = Vec.getNormalized(p00);
					n01 = Vec.getNormalized(p01);
					n10 = Vec.getNormalized(p10);
					n11 = Vec.getNormalized(p11);
					shape.addQuad(p00, p01, p10, p11, n00, n01, n10, n11);
				} else
					shape.addQuad(p00, p01, p10, p11);
			}
			p00 = Vec.getPolar(radius, parallels * parallelAngle, m * meridianAngle);
			p01 = Vec.getPolar(radius, parallels * parallelAngle, (m + 1) * meridianAngle);
			p11 = Vec.get(0, 0, -radius);
			if (smooth) {
				n00 = Vec.getNormalized(p00);
				n01 = Vec.getNormalized(p01);
				n11 = Vec.getNormalized(p11);
				shape.addTriangle(p00, p01, p11, n00, n01, n11);
			} else
				shape.addTriangle(p00, p01, p11);
		}
		
		return shape;
	}
	
	static void subdivideOnSphere(ShapeDefinition shape, float radius, int subdivisions, float[] p0, float[] p1, float[] p2) {
		if (subdivisions == 0) {
			shape.addTriangle(p0, p1, p2);
			return;
		}
		
		float[] p01, p02, p12;
		p01 = Vec.getScaled(radius, Vec.add(p0, p1));
		p02 = Vec.getScaled(radius, Vec.add(p0, p2));
		p12 = Vec.getScaled(radius, Vec.add(p1, p2));
		--subdivisions;
		
		subdivideOnSphere(shape, radius, subdivisions, p0, p01, p02);
		subdivideOnSphere(shape, radius, subdivisions, p01, p1, p12);
		subdivideOnSphere(shape, radius, subdivisions, p02, p12, p2);
		subdivideOnSphere(shape, radius, subdivisions, p01, p12, p02);
	}

	static void subdivideSmooth(ShapeDefinition shape, float radius, int subdivisions, float[] p0, float[] p1, float[] p2) {
		if (subdivisions == 0) {
			float[] n0, n1, n2;
			n0 = Vec.getNormalized(p0);
			n1 = Vec.getNormalized(p1);
			n2 = Vec.getNormalized(p2);
			shape.addTriangle(p0, p1, p2, n0, n1, n2);
			return;
		}
		
		float[] p01, p02, p12;
		p01 = Vec.getScaled(radius, Vec.add(p0, p1));
		p02 = Vec.getScaled(radius, Vec.add(p0, p2));
		p12 = Vec.getScaled(radius, Vec.add(p1, p2));
		--subdivisions;
		
		subdivideSmooth(shape, radius, subdivisions, p0, p01, p02);
		subdivideSmooth(shape, radius, subdivisions, p01, p1, p12);
		subdivideSmooth(shape, radius, subdivisions, p02, p12, p2);
		subdivideSmooth(shape, radius, subdivisions, p01, p12, p02);
	}
	
	static void subdivideOnSphere(ShapeDefinition shape, float radius, int subdivisions, float[] p00, float[] p01, float[] p10, float[] p11) {
		if (subdivisions == 0) {
			shape.addQuad(p00, p01, p10, p11);
			return;
		}
		
		float[] p0x, p1x, px0, px1, pxx;
		p0x = Vec.getScaled(radius, Vec.add(p00, p01));
		p1x = Vec.getScaled(radius, Vec.add(p10, p11));
		px0 = Vec.getScaled(radius, Vec.add(p00, p10));
		px1 = Vec.getScaled(radius, Vec.add(p01, p11));
		pxx = Vec.getScaled(radius, Vec.add(p00, p11));
		--subdivisions;
		
		subdivideOnSphere(shape, radius, subdivisions, p00, p0x, px0, pxx);
		subdivideOnSphere(shape, radius, subdivisions, p0x, p01, pxx, px1);
		subdivideOnSphere(shape, radius, subdivisions, px0, pxx, p10, p1x);
		subdivideOnSphere(shape, radius, subdivisions, pxx, px1, p1x, p11);
	}

	static void subdivideSmooth(ShapeDefinition shape, float radius, int subdivisions, float[] p00, float[] p01, float[] p10, float[] p11) {
		if (subdivisions == 0) {
			float[] n00, n01, n10, n11;
			n00 = Vec.getNormalized(p00);
			n01 = Vec.getNormalized(p01);
			n10 = Vec.getNormalized(p10);
			n11 = Vec.getNormalized(p11);
			shape.addQuad(p00, p01, p10, p11, n00, n01, n10, n11);
			return;
		}
		
		float[] p0x, p1x, px0, px1, pxx;
		p0x = Vec.getScaled(radius, Vec.add(p00, p01));
		p1x = Vec.getScaled(radius, Vec.add(p10, p11));
		px0 = Vec.getScaled(radius, Vec.add(p00, p10));
		px1 = Vec.getScaled(radius, Vec.add(p01, p11));
		pxx = Vec.getScaled(radius, Vec.add(p00, p11));
		--subdivisions;
		
		subdivideSmooth(shape, radius, subdivisions, p00, p0x, px0, pxx);
		subdivideSmooth(shape, radius, subdivisions, p0x, p01, pxx, px1);
		subdivideSmooth(shape, radius, subdivisions, px0, pxx, p10, p1x);
		subdivideSmooth(shape, radius, subdivisions, pxx, px1, p1x, p11);
	}
	
	
	public static float getSigned(float v, int index) {
		return index < 1 ? -v : v;
	}
	
	public static ShapeDefinition createSphere(float radius, int subdivisions, boolean smooth) {
		ShapeDefinition shape = new ShapeDefinition();
		
		float extent = radius / (float) Math.sqrt(3);
		float[][][][] p = new float[2][][][];
		for (int z = 0; z < 2; ++z) {
			p[z] = new float[2][][];
			for (int y = 0; y < 2; ++y) {
				p[z][y] = new float[2][];
				for (int x = 0; x < 2; ++x) { 
					p[z][y][x] = Vec.get(getSigned(extent, x), getSigned(extent, y), getSigned(extent, z)); 
				}
			}
		}
		
		if (smooth) {
			subdivideSmooth(shape, radius, subdivisions, p[0][0][0], p[0][1][0], p[1][0][0], p[1][1][0]);
			subdivideSmooth(shape, radius, subdivisions, p[1][0][1], p[1][1][1], p[0][0][1], p[0][1][1]);
			subdivideSmooth(shape, radius, subdivisions, p[1][0][0], p[1][0][1], p[0][0][0], p[0][0][1]);
			subdivideSmooth(shape, radius, subdivisions, p[0][1][0], p[0][1][1], p[1][1][0], p[1][1][1]);
			subdivideSmooth(shape, radius, subdivisions, p[0][0][0], p[0][0][1], p[0][1][0], p[0][1][1]);
			subdivideSmooth(shape, radius, subdivisions, p[1][1][0], p[1][1][1], p[1][0][0], p[1][0][1]);
			
		} else {
			subdivideOnSphere(shape, radius, subdivisions, p[0][0][0], p[0][1][0], p[1][0][0], p[1][1][0]);
			subdivideOnSphere(shape, radius, subdivisions, p[1][0][1], p[1][1][1], p[0][0][1], p[0][1][1]);
			subdivideOnSphere(shape, radius, subdivisions, p[1][0][0], p[1][0][1], p[0][0][0], p[0][0][1]);
			subdivideOnSphere(shape, radius, subdivisions, p[0][1][0], p[0][1][1], p[1][1][0], p[1][1][1]);
			subdivideOnSphere(shape, radius, subdivisions, p[0][0][0], p[0][0][1], p[0][1][0], p[0][1][1]);
			subdivideOnSphere(shape, radius, subdivisions, p[1][1][0], p[1][1][1], p[1][0][0], p[1][0][1]);
		}
		
		return shape;
	}

	public static ShapeDefinition createSphereTri(float radius, int subdivisions, boolean smooth) {
		ShapeDefinition shape = new ShapeDefinition();
		
		float phi = (float) Math.PI * 2 / 3;
		float theta = (float) (2 * Math.acos(0.5 / Math.sin(Math.PI / 3))); 
		float[] p1, p2, p3, p4;
		p1 = Vec.get(0, 0, radius);
		p2 = Vec.getPolar(radius, theta, 0);
		p3 = Vec.getPolar(radius, theta, phi);
		p4 = Vec.getPolar(radius, theta, -phi);
		
		if (smooth) {
			subdivideSmooth(shape, radius, subdivisions, p2, p1, p3);
			subdivideSmooth(shape, radius, subdivisions, p3, p1, p4);
			subdivideSmooth(shape, radius, subdivisions, p4, p1, p2);
			subdivideSmooth(shape, radius, subdivisions, p4, p2, p3);
		} else {
			subdivideOnSphere(shape, radius, subdivisions, p2, p1, p3);
			subdivideOnSphere(shape, radius, subdivisions, p3, p1, p4);
			subdivideOnSphere(shape, radius, subdivisions, p4, p1, p2);
			subdivideOnSphere(shape, radius, subdivisions, p4, p2, p3);
		}

		return shape;
	}
}
