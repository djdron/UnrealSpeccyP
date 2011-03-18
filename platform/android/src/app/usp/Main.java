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
import android.os.Bundle;
import android.widget.LinearLayout;
import android.content.Context;
import android.content.res.Configuration;

public class Main extends Activity
{
	ByteBuffer getBinResource(int id)
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
	LinearLayout layout;
	View view;
	Control control;
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		ByteBuffer font = getBinResource(R.raw.spxtrm4f);
		ByteBuffer rom0 = getBinResource(R.raw.sos128);
		ByteBuffer rom1 = getBinResource(R.raw.sos48);
		ByteBuffer rom2 = getBinResource(R.raw.service);
		ByteBuffer rom3 = getBinResource(R.raw.dos513f);
		Emulator.the.Init(font, rom0, rom1, rom2, rom3);
		Context c = getApplicationContext();
		view = new View(c);
		control = new Control(c);
		layout = new LinearLayout(c);
		setContentView(layout);
		UpdateOrientation(getResources().getConfiguration());
    }
	public void UpdateOrientation(Configuration config)
	{
		layout.removeAllViews();
		if(config.orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
			layout.setOrientation(LinearLayout.HORIZONTAL);
			layout.addView(control);
			layout.addView(view);
		}
		else
		{
			layout.setOrientation(LinearLayout.VERTICAL);
			layout.addView(view);
			layout.addView(control);
		}
		control.requestFocus();
	}
    protected void onPause()
	{
    	super.onPause();
    	view.OnPause();
	}
    protected void onResume()
	{
    	super.onResume();
    	view.OnResume();
	}
    @Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		UpdateOrientation(newConfig);
	}
}
