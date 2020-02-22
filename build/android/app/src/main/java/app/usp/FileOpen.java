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

import android.Manifest;
import android.app.AlertDialog;
import android.app.TabActivity;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.MenuItem;
import android.widget.TabHost;
import app.usp.fs.FileSelectorFS;
import app.usp.fs.FileSelectorRZX;
import app.usp.fs.FileSelectorVtrdos;
import app.usp.fs.FileSelectorWOS;
import app.usp.fs.FileSelectorBBB;

public class FileOpen extends TabActivity
{
	private static int active_tab = 0;
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		getActionBar().setDisplayHomeAsUpEnabled(true);
		setContentView(R.layout.file_open);

		Resources res = getResources();
		TabHost tabHost = getTabHost();
		Intent intent = new Intent().setClass(this, FileSelectorFS.class);
		TabHost.TabSpec spec = tabHost.newTabSpec("file_select_fs")
								.setIndicator(res.getString(R.string.file_system))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorVtrdos.class);
		spec = tabHost.newTabSpec("file_select_vtrdos")
								.setIndicator(res.getString(R.string.vtrdos))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorWOS.class);
		spec = tabHost.newTabSpec("file_select_wos")
								.setIndicator(res.getString(R.string.wos))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorRZX.class);
		spec = tabHost.newTabSpec("file_select_rzx")
								.setIndicator(res.getString(R.string.rzx))
								.setContent(intent);
		tabHost.addTab(spec);

		intent = new Intent().setClass(this, FileSelectorBBB.class);
		spec = tabHost.newTabSpec("file_select_bbb")
								.setIndicator(res.getString(R.string.bbb))
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
	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch(item.getItemId())
		{
		case android.R.id.home:	onBackPressed(); break;
		}
		return super.onOptionsItemSelected(item);
	}

	static final int RP_STORAGE = 0;
	FileSelectorFS permission_requestor;
	public boolean CheckStoragePermission()
	{
		return android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.M ||
				checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE) == android.content.pm.PackageManager.PERMISSION_GRANTED;
	}
	public void RequestStoragePermission(FileSelectorFS fs)
	{
		if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
		{
			permission_requestor = fs;
			requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, RP_STORAGE);
		}
	}
	@Override
	public void onRequestPermissionsResult(int code, String[] permissions, int[] grants)
	{
		super.onRequestPermissionsResult(code, permissions, grants);
		if(code == RP_STORAGE)
		{
			if(grants[0] == android.content.pm.PackageManager.PERMISSION_GRANTED)
				permission_requestor.UpdateAll();
			else
			{
				permission_requestor.ResetToRoot();
				AlertDialog.Builder dlg = new AlertDialog.Builder(this);
				dlg.setMessage(getString(R.string.unable_access_storage));
				dlg.setCancelable(true);
				dlg.setPositiveButton(getString(R.string.preferences),
					new DialogInterface.OnClickListener()
					{
						@Override
						public void onClick(DialogInterface di, int i)
						{
							startActivity(new Intent(android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS, Uri.fromParts("package", getPackageName(), null)));
						}
					}
				);
				dlg.setNegativeButton(getString(R.string.cancel), null);
				AlertDialog ad = dlg.create();
				ad.show();
			}
		}
		permission_requestor = null;
	}
}
