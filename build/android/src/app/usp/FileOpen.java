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

import android.app.TabActivity;
import android.content.Intent;
import android.content.res.Resources;
import android.os.Bundle;
import android.widget.TabHost;
import app.usp.fs.FileSelectorFS;
import app.usp.fs.FileSelectorRZX;
import app.usp.fs.FileSelectorVtrdos;
import app.usp.fs.FileSelectorWOS;

public class FileOpen extends TabActivity
{
	private static int active_tab = 0;
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.file_open);

		Resources res = getResources();
		TabHost tabHost = getTabHost();
		Intent intent = new Intent().setClass(this, FileSelectorFS.class);
		TabHost.TabSpec spec = tabHost.newTabSpec("file_select_fs")
								.setIndicator(res.getString(R.string.file_system), res.getDrawable(R.drawable.icon_tab_file))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorVtrdos.class);
		spec = tabHost.newTabSpec("file_select_vtrdos")
								.setIndicator(res.getString(R.string.vtrdos), res.getDrawable(R.drawable.icon_tab_vtrdos))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorWOS.class);
		spec = tabHost.newTabSpec("file_select_wos")
								.setIndicator(res.getString(R.string.wos), res.getDrawable(R.drawable.icon_tab_wos))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorRZX.class);
		spec = tabHost.newTabSpec("file_select_rzx")
								.setIndicator(res.getString(R.string.rzx), res.getDrawable(R.drawable.icon_tab_rzx))
								.setContent(intent);
		tabHost.addTab(spec);

	    tabHost.setCurrentTab(active_tab);
	}
    @Override
	public void onDestroy()
	{
    	active_tab = getTabHost().getCurrentTab();
    	super.onDestroy();
	}
}
