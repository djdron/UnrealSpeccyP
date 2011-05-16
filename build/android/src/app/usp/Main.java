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
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
//import com.mobclix.android.sdk.MobclixAdView;
//import com.mobclix.android.sdk.MobclixMMABannerXLAdView;

public class Main extends Activity
{
	private TableLayout layout;
	private TableRow row1, row2;
	private app.usp.View view;
	private Control control;
//	private MobclixAdView banner;
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
//		banner = new MobclixMMABannerXLAdView(this);
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
		row2.setMinimumHeight(80);
		if(config.orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
			row1.setGravity(Gravity.CENTER);
			row2.setGravity(Gravity.RIGHT|Gravity.BOTTOM);
			row1.addView(control, new TableRow.LayoutParams());
			row1.addView(view, new TableRow.LayoutParams());
			row2.addView(view_dummy, new TableRow.LayoutParams());
//			row2.addView(banner, new TableRow.LayoutParams());
			layout.addView(row1);
			layout.addView(row2);
		}
		else
		{
			row2.setGravity(Gravity.BOTTOM);
			layout.addView(view);
			layout.addView(control);
//			row2.addView(banner, new TableRow.LayoutParams());
			layout.addView(row2);
		}
		control.requestFocus();
		view.setKeepScreenOn(true);
	}
    @Override
	protected void onResume()
	{
		super.onResume();
		view.OnResume();
	}
    @Override
	protected void onPause()
	{
		Emulator.the.StoreOptions();
		view.OnPause();
		super.onPause();
	}
    @Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		UpdateOrientation(newConfig);
	}
    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
    	getMenuInflater().inflate(R.menu.menu, menu);		
    	return true;
    }
    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
    	switch(item.getItemId())
    	{
    	case R.id.open_file:	startActivityForResult(new Intent(this, FileSelector.class), 0); return true;
		case R.id.save_state:	Emulator.the.SaveState(); 		return true;
		case R.id.load_state:	Emulator.the.LoadState(); 		return true;
		case R.id.reset:		Emulator.the.Reset(); 			return true;
    	case R.id.preferences:	startActivityForResult(new Intent(this, Preferences.class), 0); return true;
		case R.id.quit:			finish(); 						return true;    		
    	}
    	return super.onOptionsItemSelected(item);
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
