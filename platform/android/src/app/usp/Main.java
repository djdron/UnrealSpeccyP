/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

package app.usp;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.Toast;
import android.view.Gravity;
import android.view.View;
import android.content.Context;
import android.content.res.Configuration;
import com.mobclix.android.sdk.MobclixAdView;
import com.mobclix.android.sdk.MobclixMMABannerXLAdView;

public class Main extends Activity
{
	private TableLayout layout;
	private TableRow row1, row2;
	private app.usp.View view;
	private Control control;
	private MobclixAdView banner;
	private View view_dummy;
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		Emulator.the.InitResources(BinRes(R.raw.spxtrm4f), BinRes(R.raw.sos128), BinRes(R.raw.sos48), BinRes(R.raw.service), BinRes(R.raw.dos513f));
		Emulator.the.Init(getFilesDir().getAbsolutePath());
		Context c = getApplicationContext();
		view = new app.usp.View(this, c);
		control = new Control(c);
		banner = new MobclixMMABannerXLAdView(this);
		layout = new TableLayout(c);
		row1 = new TableRow(c);
		row2 = new TableRow(c);
		view_dummy = new View(c);
		setContentView(layout);
		UpdateOrientation(getResources().getConfiguration());
		String file = Uri.parse(getIntent().toUri(0)).getPath();
		if(file.length() != 0)
		{
			Toast.makeText(getApplicationContext(), "Opening \"" + file + "\"", Toast.LENGTH_LONG).show();
			Emulator.the.Open(file);
		}
//		banner.setTestMode(true);
    }
    @Override
    public void onDestroy()
    {
    	Emulator.the.Done();
    	super.onDestroy();
    }
	private void UpdateOrientation(Configuration config)
	{
		row1.removeAllViews();
		row2.removeAllViews();
		layout.removeAllViews();
		if(config.orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
			row1.setGravity(Gravity.CENTER);
			row2.setGravity(Gravity.RIGHT);
			row1.addView(control, new TableRow.LayoutParams());
			row1.addView(view, new TableRow.LayoutParams());
			row2.addView(view_dummy, new TableRow.LayoutParams());
			row2.addView(banner, new TableRow.LayoutParams());
			layout.addView(row1);
			layout.addView(row2);
		}
		else
		{
			row1.setGravity(Gravity.BOTTOM);
			layout.addView(view);
			layout.addView(control);
			row1.addView(banner, new TableRow.LayoutParams());
			layout.addView(row1);
		}
		control.requestFocus();
	}
    protected void onPause()
	{
    	Emulator.the.StoreOptions();
    	super.onPause();
	}
    @Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		UpdateOrientation(newConfig);
	}
	final private ByteBuffer BinRes(int id)
	{
		InputStream is = getResources().openRawResource(id);
		byte[] data = null;
		try
		{
			data = new byte[is.available()];
			is.read(data);
		}
		catch(IOException e)
		{}
		ByteBuffer bb = ByteBuffer.allocateDirect(data.length);
		bb.put(data);
		bb.rewind();
		return bb;
	}
}
