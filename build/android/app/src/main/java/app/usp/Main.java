/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import java.lang.reflect.Method;
import java.nio.ByteBuffer;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.widget.RelativeLayout;
import android.widget.Toast;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.content.Context;
import android.content.Intent;
import android.app.Activity;
import app.usp.ctl.Control;

public class Main extends Activity
{
	private ViewGLES view;
	private Control control;
	private Handler hide_callback;
	private Runnable hide_runnable;
	private boolean paused = false;

	@Override
    public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		Emulator.the.InitRom(0, BinRes(R.raw.sos128_0));
		Emulator.the.InitRom(1, BinRes(R.raw.sos128_1));
		Emulator.the.InitRom(2, BinRes(R.raw.sos48));
		Emulator.the.InitRom(3, BinRes(R.raw.service));
		Emulator.the.InitRom(4, BinRes(R.raw.dos513f));
		Emulator.the.InitFont(BinRes(R.raw.spxtrm4f));
		Emulator.the.Init(getFilesDir().getAbsolutePath());
		Context c = getApplicationContext();
		view = new ViewGLES(c);
		view.setId(1);
		control = new Control(c, view);
		control.setId(2);
		RelativeLayout layout = new RelativeLayout(c);
		RelativeLayout.LayoutParams p1 = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
		RelativeLayout.LayoutParams p2 = new RelativeLayout.LayoutParams(RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.WRAP_CONTENT);
		p1.addRule(RelativeLayout.ABOVE, control.getId());
		p1.addRule(RelativeLayout.CENTER_HORIZONTAL);
		p2.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
		p2.addRule(RelativeLayout.CENTER_HORIZONTAL);
		layout.addView(view, p1);
		layout.addView(control, p2);
		setContentView(layout);

		if(AbleImmersiveMode())
		{
			getWindow().getDecorView().setOnSystemUiVisibilityChangeListener(
				new View.OnSystemUiVisibilityChangeListener()
				{
					@Override
					public void onSystemUiVisibilityChange(int visibility)
					{
						if((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0)
						{
							RunHideCallback();
						}
					}
				}
			);
		}

		control.requestFocus();
		view.setKeepScreenOn(true);
		String file = Uri.parse(getIntent().toUri(0)).getPath();
		if(file.length() != 0)
		{
			Toast.makeText(getApplicationContext(), String.format(getString(R.string.opening), file), Toast.LENGTH_LONG).show();
			Emulator.the.Open(file);
		}
    }
	static boolean AbleImmersiveMode()
	{
		return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT;
	}
	static boolean AbleActionBar()
	{
		return android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT;
	}
	private void RunHideCallback()
	{
		CancelHideCallback();
		BeginPause();
		hide_runnable = new Runnable()
		{
			@Override
			public void run() { HideSystemUI(); }
		};
		hide_callback = new Handler(getApplicationContext().getMainLooper());
		hide_callback.postDelayed(hide_runnable, 3000);
	}
	private void CancelHideCallback()
	{
		if(hide_callback != null)
		{
			hide_callback.removeCallbacks(hide_runnable);
			hide_callback = null;
			hide_runnable = null;
		}
	}
	private void HideSystemUI()
	{
		CancelHideCallback();
		EndPause();
		if(!AbleImmersiveMode())
			return;
		getWindow().getDecorView().setSystemUiVisibility(
				View.SYSTEM_UI_FLAG_LAYOUT_STABLE
						| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
						| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
						| View.SYSTEM_UI_FLAG_FULLSCREEN
						| View.SYSTEM_UI_FLAG_IMMERSIVE);
	}
	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		super.onWindowFocusChanged(hasFocus);
		if(hasFocus)
			HideSystemUI();
	}
    @Override
	public void onDestroy()
	{
		CancelHideCallback();
		Emulator.the.Done();
		super.onDestroy();
    	android.os.Process.killProcess(android.os.Process.myPid());
	}
    @Override
	protected void onResume()
	{
		super.onResume();
		view.OnResume();
		control.OnResume();
		EndPause();
	}
    @Override
	protected void onPause()
	{
		Emulator.the.StoreOptions();
		view.OnPause();
		control.OnPause();
		super.onPause();
	}

	private void BeginPause()
	{
		if(!paused)
		{
			paused = true;
			Emulator.the.VideoPaused(true);
		}
	}
	private void EndPause()
	{
		if(paused)
		{
			paused = false;
			Emulator.the.VideoPaused(false);
		}
	}

    @Override
    public boolean onCreateOptionsMenu(Menu menu)
    {
		getMenuInflater().inflate(R.menu.menu, menu);
		return super.onCreateOptionsMenu(menu);
    }
	@Override
	public boolean onPrepareOptionsMenu(Menu menu)
	{
		CancelHideCallback();
		BeginPause();
		return super.onPrepareOptionsMenu(menu);
	}
	@Override
	public void onOptionsMenuClosed(Menu menu)
	{
		EndPause();
		super.onOptionsMenuClosed(menu);
	}
	static final int A_FILE_SELECTOR = 0;
    static final int A_PREFERENCES = 1;
    @Override
    public boolean onOptionsItemSelected(MenuItem item)
    {
		EndPause();
    	switch(item.getItemId())
    	{
    	case R.id.open_file:	startActivityForResult(new Intent(this, FileOpen.class), A_FILE_SELECTOR); return true;
		case R.id.save_state:	Emulator.the.SaveState(); 		return true;
		case R.id.load_state:	Emulator.the.LoadState(); 		return true;
		case R.id.reset:		Emulator.the.Reset(); 			return true;
    	case R.id.preferences:	startActivityForResult(new Intent(this, Preferences.class), A_PREFERENCES); return true;
		case R.id.quit:			Exit(); 						return true;
    	}
    	return super.onOptionsItemSelected(item);
    }
	final private void Exit()
	{
    	finish();
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
