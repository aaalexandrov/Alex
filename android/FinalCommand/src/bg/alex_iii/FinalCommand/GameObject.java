package bg.alex_iii.FinalCommand;

public interface GameObject {
	public interface Def {
		public Class<? extends GameObject> gameObjectClass();
	}
	
	public boolean init(Game game, Def def);
	public boolean render();
	public void update();
	public float[] getPosition(float[] position);
	public void setPosition(float x, float y, float z);
}
