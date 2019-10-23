package bg.alex_iii.GLES;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

public class GLESCompareSorter implements GLESSorter {
	Comparator<GLESModel> mComparator;
	ArrayList<GLESModel> mModels;
	
	public GLESCompareSorter(Comparator<GLESModel> comparator) {
		mComparator = comparator;
		assert mComparator != null;
		mModels = new ArrayList<GLESModel>(); 
	}
	
	public boolean add(GLESModel model) {
		mModels.add(model);
		return true;
	}

	public boolean sortAndRender() {
		Collections.sort(mModels, mComparator);
		boolean result = true;
		for (GLESModel m: mModels)
			result &= m.render();
		mModels.clear();
		return result;
	}

}
