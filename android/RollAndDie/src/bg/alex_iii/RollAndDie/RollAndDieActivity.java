package bg.alex_iii.RollAndDie;

import java.util.Random;

import android.app.Activity;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;

public class RollAndDieActivity extends Activity {
	static final int DICE_MIN = 1;
	static final int DICE_MAX = 999;
	static final int LINES_MAX = 64;
	
	EditText mDice;
	Spinner mSides, mLogType, mSuccessThreshold, mDoubleThreshold;
	TextView mResults;
	Random mRandom = new Random();
	String mSuccessOn = "Success on ";
	String mDoubleOn = "Double on ";
	String mDontDouble = "Don't double";
	String[] mSuccessStrings, mDoubleStrings;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);        
        setContentView(R.layout.main);
        mDice = (EditText) findViewById(R.id.dice_number);
        mSides = (Spinner) findViewById(R.id.die_sides);
        mSuccessThreshold = (Spinner) findViewById(R.id.success_threshold);
        mDoubleThreshold = (Spinner) findViewById(R.id.double_threshold);
        mLogType = (Spinner) findViewById(R.id.log_type);
        mSides.setOnItemSelectedListener(new OnItemSelectedListener() {
        	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        		onDieSides(parent, position);
        	}
        	public void onNothingSelected(AdapterView<?> parent) {
        	}
        });
        mSuccessThreshold.setOnItemSelectedListener(new OnItemSelectedListener() {
        	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        		onSuccessThreshold(parent, position);
        	}
        	public void onNothingSelected(AdapterView<?> parent) {
        	}
        });
        mLogType.setOnItemSelectedListener(new OnItemSelectedListener() {
        	public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        		onLogType(parent, position);
        	}
        	public void onNothingSelected(AdapterView<?> parent) {
        	}
        });
        mResults = (TextView) findViewById(R.id.results);
        mResults.setMovementMethod(new ScrollingMovementMethod());
        initStrings();
        loadPreferences();
    }
    
    @Override
    public void onStop() {
    	savePreferences();
    	super.onStop();
    }

    public void initStrings() {
    	String die = (String) mSides.getItemAtPosition(mSides.getCount() - 1);
    	die = die.substring(1);
    	int dieSides = Integer.parseInt(die);
    	mSuccessStrings = new String[dieSides];
    	mDoubleStrings = new String[dieSides];
    	for (int i = 0; i < dieSides; i++) {
    		mSuccessStrings[i] = mSuccessOn + Integer.toString(i + 1);
    		mDoubleStrings[i] = mDoubleOn + Integer.toString(i + 1);
    	}
    }
    
    public void populateSuccessThreshold(int threshold) {
    	if (mSuccessThreshold.getVisibility() != View.VISIBLE)
    		return;
    	int dieSides = getDieSides();
    	threshold = Math.min(Math.max(1, threshold), dieSides);
    	String[] values = new String[dieSides];
    	for (int i = 0; i < dieSides; i++) {
    		values[i] = mSuccessStrings[i];
    	}
    	ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item, values);
    	adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    	mSuccessThreshold.setAdapter(adapter);
    	mSuccessThreshold.setSelection(threshold - 1);
    }

    public int getSuccessThreshold() {
    	String threshold = (String) mSuccessThreshold.getSelectedItem();
    	threshold = threshold.substring(mSuccessOn.length());
    	return Integer.parseInt(threshold);
    }
    
    public void populateDoubleThreshold(int threshold) {
    	if (mDoubleThreshold.getVisibility() != View.VISIBLE)
    		return;
    	int dieSides = getDieSides();
    	int successThreshold = getSuccessThreshold();
    	if (threshold != 0)
    		threshold = Math.min(Math.max(successThreshold, threshold), dieSides);
    	int valCount = dieSides - successThreshold + 2;
    	String[] values = new String[valCount];
    	values[0] = mDontDouble;
    	for (int i = 1; i < valCount; i++) {
    		values[i] = mDoubleStrings[successThreshold + i - 2];
    	}
    	ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(this, android.R.layout.simple_spinner_item, values);
    	adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    	mDoubleThreshold.setAdapter(adapter);
    	if (threshold != 0)
    		mDoubleThreshold.setSelection(threshold - successThreshold + 1);
    	else
    		mDoubleThreshold.setSelection(0);
    }

    public int getDoubleThreshold() {
    	String threshold = (String) mDoubleThreshold.getSelectedItem();
    	if (threshold.equals(mDontDouble))
    		return 0;
    	threshold = threshold.substring(mDoubleOn.length());
    	return Integer.parseInt(threshold);
    }
    
    public int getDiceNumber() {
    	String diceText = mDice.getText().toString();
    	int dice;
    	try {
    		dice = Integer.parseInt(diceText);
    	} catch (NumberFormatException e) {
    		dice = DICE_MIN;
    	}
    	dice = Math.min(Math.max(DICE_MIN, dice), DICE_MAX);
    	return dice;
    }

    public void setDiceNumber(int dice) {
    	dice = Math.min(Math.max(DICE_MIN, dice), DICE_MAX); 
    	mDice.setText(Integer.toString(dice));
        mDice.setSelection(mDice.getText().length());
    }
    
    public int getDieSides() {
    	String die = (String) mSides.getSelectedItem();
    	die = die.substring(1);
    	return Integer.parseInt(die);
    }
    
    public void limitResultsLines() {
    	while (mResults.getLineCount() > LINES_MAX) {
    		CharSequence text = mResults.getText();
    		int i = 0;
    		while (text.charAt(i) != '\n')
    			i++;
    		if (i >= text.length() - 1)
    			break;
    		mResults.setText(text.subSequence(i + 1, text.length()));
    	}
    }
    
    public CharSequence rollSum(int sides, int dice) {
    	int sum = 0;
    	for (int i = 0; i < dice; i++)
    		sum += mRandom.nextInt(sides) + 1;
    	StringBuffer result = new StringBuffer(); 
    	result.append("Rolling ").append(dice).append("d").append(sides).append(" -> ").append(sum).append("\n");
    	return result;
    }

    public CharSequence rollGrouped(int sides, int dice) {
    	StringBuffer result = new StringBuffer();
    	result.append("Rolling ").append(dice).append("d").append(sides).append(" -> ");
    	int[] sideGroups = new int[sides];
    	int i, sum = 0;
    	for (i = 0; i < sides; i++)
    		sideGroups[i] = 0;
    	for (int roll = 0; roll < dice; roll++) {
    		int die = mRandom.nextInt(sides);
    		sideGroups[die]++;
    		sum += die + 1;
    	}
    	boolean first = true;
    	for (i = 0; i < sides; i++) {
    		if (sideGroups[i] == 0)
    			continue;
    		if (!first) 
    			result.append("+");
    		else
    			first = false;
    		result.append(sideGroups[i]).append("x").append(i + 1);
    	}
    	result.append(" = ").append(sum).append("\n");
    	return result;
    }

    public CharSequence rollFull(int sides, int dice) {
    	StringBuffer result = new StringBuffer();
    	result.append("Rolling ").append(dice).append("d").append(sides).append(" -> ");
    	boolean first = true;
    	int sum = 0;
    	for (int i = 0; i < dice; i++) {
    		if (!first) 
    			result.append("+");
    		else
    			first = false;
    		int die = mRandom.nextInt(sides) + 1;
    		result.append(die);
    		sum += die;
    	}
    	result.append(" = ").append(sum).append("\n");
    	return result;
    }
    
    public CharSequence rollSuccess(int sides, int dice, int successOn, int doubleOn) {
    	StringBuffer result = new StringBuffer(); 
    	String thresh;
    	if (doubleOn != 0)
    		thresh = " (>=" + Integer.toString(successOn) + "/" + Integer.toString(doubleOn) + "x2)";
    	else
    		thresh = " (>=" + Integer.toString(successOn)+ ")";
    	result.append("Rolling ").append(dice).append("d").append(sides).append(thresh).append(" -> ");
    	int successes = 0;
    	boolean hasOne = false;
    	for (int i = 0; i < dice; i++) {
    		int die = mRandom.nextInt(sides) + 1;
    		if (die >= successOn)
    			successes++;
    		if (doubleOn != 0 && die >= doubleOn)
    			successes++;
    		if (die == 1)
    			hasOne = true;
    	}
    	result.append(successes);
    	if (successes == 0 && hasOne)
    		result.append(" (rolled 1)");
    	result.append("\n");
    	return result;
    }
    
    public void onPlus(View v) {
    	setDiceNumber(getDiceNumber() + 1);
    }
    
    public void onMinus(View v) {
    	setDiceNumber(getDiceNumber() - 1);
    }

    public void onDieSides(View v, int selected) {
    	populateSuccessThreshold(getSuccessThreshold());
    	populateDoubleThreshold(getDoubleThreshold());
    }

    public void onSuccessThreshold(View v, int selected) {
    	populateDoubleThreshold(getDoubleThreshold());
    }

    public void onLogType(View v, int selected) {
    	String logType = (String) mLogType.getSelectedItem();
    	if (logType.equals("Success")) {
    		mSuccessThreshold.setVisibility(View.VISIBLE);
    		mDoubleThreshold.setVisibility(View.VISIBLE);
        	populateSuccessThreshold(getSuccessThreshold());
        	populateDoubleThreshold(getDoubleThreshold());
    	} else {
    		mSuccessThreshold.setVisibility(View.INVISIBLE);
    		mDoubleThreshold.setVisibility(View.INVISIBLE);
    	}
    }
    
    public void onRoll(View v) {
    	int sides = getDieSides();
    	int dice = getDiceNumber();
    	int successOn = getSuccessThreshold();
    	int doubleOn = getDoubleThreshold();
    	setDiceNumber(dice);
    	CharSequence result;
    	String logType = (String) mLogType.getSelectedItem();
   		if (logType.equals("Sum"))
   			result = rollSum(sides, dice);
   		else if (logType.equals("Grouped"))
    		result = rollGrouped(sides, dice);
    	else if (logType.equals("Full"))
   			result = rollFull(sides, dice);
   		else
   			result = rollSuccess(sides, dice, successOn, doubleOn);
    	mResults.append(result);
    	limitResultsLines();
    	mResults.bringPointIntoView(mResults.getText().length() - 1);
    }
    
    public void onClear(View v) {
    	mResults.setText("");
    }
    
    public boolean setSpinnerSelection(Spinner spinner, String value) {
    	for (int i = 0; i < spinner.getCount(); i++) {
    		String item = (String) spinner.getItemAtPosition(i);
    		if (item.equals(value)) {
    			spinner.setSelection(i);
    			return true;
    		}
    	}
    	return false;
    }
    
    public void savePreferences() {
    	SharedPreferences prefs = getPreferences(MODE_PRIVATE);
    	SharedPreferences.Editor prefEditor = prefs.edit();
    	int sides = getDieSides();
    	int dice = getDiceNumber();
    	int successThreshold = getSuccessThreshold();
    	int doubleThreshold = getDoubleThreshold();
    	String logType = (String) mLogType.getSelectedItem();
    	prefEditor.putInt("die_sides", sides);
    	prefEditor.putInt("dice", dice);
    	prefEditor.putInt("success_threshold", successThreshold);
    	prefEditor.putInt("double_threshold", doubleThreshold);
    	prefEditor.putString("log_type", logType);
    	prefEditor.commit();
    }
    
    public void loadPreferences() {
    	SharedPreferences prefs = getPreferences(MODE_PRIVATE);
    	int sides = prefs.getInt("die_sides", 6);
    	int dice = prefs.getInt("dice", DICE_MIN);
    	int successThreshold = prefs.getInt("success_threshold", sides / 2 + 1);
    	int doubleThreshold = prefs.getInt("double_threshold", sides);
    	String logType = prefs.getString("log_type", "Sum");
    	setSpinnerSelection(mSides, "d" + sides);
    	setDiceNumber(dice);
    	setSpinnerSelection(mLogType, logType);
    	populateSuccessThreshold(successThreshold);
    	populateDoubleThreshold(doubleThreshold);
    	onLogType(mLogType, mLogType.getSelectedItemPosition());
    }
}