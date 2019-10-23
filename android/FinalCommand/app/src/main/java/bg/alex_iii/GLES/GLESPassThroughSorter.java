package bg.alex_iii.GLES;

public class GLESPassThroughSorter implements GLESSorter {
	public boolean add(GLESModel model) {
		return model.render();
	}

	public boolean sortAndRender() {
		return true;
	}
}
