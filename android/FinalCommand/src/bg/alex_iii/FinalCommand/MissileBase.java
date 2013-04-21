package bg.alex_iii.FinalCommand;

import android.opengl.Matrix;
import bg.alex_iii.GLES.GLESModel;
import bg.alex_iii.GLES.GLESSorter;
import bg.alex_iii.GLES.Shape;
import bg.alex_iii.GLES.Vec;

public class MissileBase implements GameObject {
	public static final Def DEF = new Def();
	
	public Game mGame;
	public GLESModel mModel;
	
	public static class Def implements GameObject.Def {
		public Class<? extends GameObject> gameObjectClass() {
			return MissileBase.class;
		}
	}
	
	public boolean init(Game game, GameObject.Def def) {
		mGame = game;

		GLESModel original = mGame.mMainRenderer.mCone;
		float[] transform = new float[16];
		Matrix.setIdentityM(new float[16], 0);
		mModel = new GLESModel(original.mGeometry, original.mMaterial, transform);
		
		return true;
	}

	public boolean render() {
		return mModel.render();
	}

	public boolean addToSorter(GLESSorter sorter) {
		return sorter.add(mModel);
	}
	
	public void update() {
	}

	public boolean isPointInside(float[] point) {
		float curX, curY, curZ;
		curX = mModel.mTransform[12];
		curY = mModel.mTransform[13];
		curZ = mModel.mTransform[14];
		return Shape.isPointInsideCone(point, Vec.get(curX, curY, curZ), GameSettings.BASE_HEIGHT, GameSettings.BASE_RADIUS);
	}
	
	public float[] getPosition(float[] position) {
		position[0] = mModel.mTransform[12];
		position[1] = mModel.mTransform[13];
		position[2] = mModel.mTransform[14];
		return position;
	}
	
	
	public void setPosition(float x, float y, float z) {
		Matrix.setIdentityM(mModel.mTransform, 0);
		Matrix.translateM(mModel.mTransform, 0, x, y, z);
	}
}
