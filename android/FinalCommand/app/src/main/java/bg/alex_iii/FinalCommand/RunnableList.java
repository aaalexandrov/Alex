package bg.alex_iii.FinalCommand;

import java.util.ArrayList;

public class RunnableList {
	ArrayList<Runnable> mRunnables = new ArrayList<Runnable>();
	
	public synchronized void add(Runnable runnable) {
		mRunnables.add(runnable);
	}
	
	public synchronized void run() {
		for (Runnable r: mRunnables)
			r.run();
		mRunnables.clear();
	}
}
