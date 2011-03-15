package app.usp;

import android.app.Activity;
import android.os.Bundle;
import android.widget.LinearLayout;
import android.content.Context;

public class Main extends Activity
{
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		Emulator.the.Init();
		Context c = getApplicationContext();
		LinearLayout v = new LinearLayout(c);
		v.setOrientation(LinearLayout.VERTICAL);
		v.addView(new View(c));
		v.addView(new Control(c));
		setContentView(v);
    }
}
