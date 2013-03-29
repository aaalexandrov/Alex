package bg.alex_iii.FinalCommand;

import bg.alex_iii.GLES.GLESModel;

public class Missile implements GameObject {
	Game mGame;
	GLESModel mModel;
	
	Missile(Game game) {
		mGame = game;
	}
	
	public Game getGame() {
		return mGame;
	}
	
	public boolean render() {
		return mModel.render();
	}
	
	public void update() {
		
	}
}
