package bg.alex_iii.FinalCommand;

public interface GameObject {
	public Game getGame();
	public boolean render();
	public void update();
	public void setPosition(float x, float y, float z);
}
