package bg.alex_iii.GLES;

public interface GLESUserRenderer {
	public void setRenderer(GLESRenderer renderer);
	public GLESRenderer getRenderer();
	public void setDimensions(int width, int height);
	public boolean init();
	public void update();
	public boolean render();
}
