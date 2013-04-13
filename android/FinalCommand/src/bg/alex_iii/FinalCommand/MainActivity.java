package bg.alex_iii.FinalCommand;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends Activity {

    public MainView mMainView;
    public TextView mStatusLabel;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mMainView = (MainView) findViewById(R.id.gLESView1);
        mStatusLabel = (TextView) findViewById(R.id.statusLabel);
    }
    
    public void setStatusText(String text) {
    	mStatusLabel.setText(text);
    }
}