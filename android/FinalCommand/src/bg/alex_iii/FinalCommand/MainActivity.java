package bg.alex_iii.FinalCommand;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {

    public MainView mCubeView;
    public TextView mStatusLabel;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mCubeView = (MainView) findViewById(R.id.gLESView1);
        mStatusLabel = (TextView) findViewById(R.id.statusLabel);
//        mGLESView = new GLESView(this);
//        setContentView(mGLESView);
    }
    
    public void setStatusText(String text) {
    	mStatusLabel.setText(text);
    }
}