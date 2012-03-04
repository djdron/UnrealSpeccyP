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

package app.usp.fs;

import java.io.File;
import java.util.ArrayList;
import java.util.List;


import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Context;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.os.AsyncTask;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import app.usp.R;

public abstract class FileSelector extends ListActivity
{
	public static class State
	{
		File current_path = new File("/");
		String last_name = "";
		List<FileSelectorSource.Item> items = new ArrayList<FileSelectorSource.Item>();
	}
	abstract State State();
	List<FileSelectorSource.Item> Items() { return State().items; }
	abstract boolean LongUpdate();

	private boolean async_task = false;

	int PathLevel(final File path)
	{
		File p = path;
		int l = 0;
		while((p = p.getParentFile()) != null)
			++l;
		return l;
	}

	protected List<FileSelectorSource> sources = new ArrayList<FileSelectorSource>();
	@Override
	protected void onResume()
	{
		super.onResume();
		if(Items().size() > 0)
		{
			SetItems();
			SelectItem(State().last_name);
		}
		else
		{
			new UpdateAsync(this, State().last_name).execute();
		}
	}
	private void SetItems()
	{
		setListAdapter(new MyListAdapter(this, new ArrayList<FileSelectorSource.Item>(Items())));
	}
	private void SelectItem(final String name)
	{
		if(name.length() > 0)
		{
			int idx = 0;
			for(FileSelectorSource.Item i : Items())
			{
				if(i.name.equals(name))
				{
					getListView().setSelection(idx);
					break;
				}
				++idx;
			}
		}
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		if(async_task)
			return;

		String f = Items().get(position).name;
		if(f.equals("/.."))
		{
			File parent = State().current_path.getParentFile();
			if(parent != null)
			{
				String name = "/" + State().current_path.getName();
				State().current_path = parent;
				new UpdateAsync(this, name).execute();
			}
		}
		else
		{
			if(f.startsWith("/"))
			{
				State().current_path = new File(State().current_path.getPath() + f);
				new UpdateAsync(this, "").execute();
			}
			else
			{
				State().last_name = f;
				new ApplyAsync(this, Items().get(position)).execute();
			}
		}
	}

	private class ApplyAsync extends AsyncTask<Void, Void, FileSelectorSource.ApplyResult>
	{
		private FileSelector owner;
		FileSelectorSource.Item item;
		ProgressDialog progress_dialog = null;
		ApplyAsync(FileSelector _owner, FileSelectorSource.Item _item)
		{
			owner = _owner;
			item = _item;
		}
		@Override
		protected void onPreExecute()
		{
			async_task = true;
			if(LongUpdate())
				progress_dialog = ProgressDialog.show(owner, getString(R.string.accessing_web), getString(R.string.downloading_image));
		}
		@Override
		protected FileSelectorSource.ApplyResult doInBackground(Void... args)
		{
			for(FileSelectorSource s : sources)
			{
				return s.ApplyItem(item);
			}
			return FileSelectorSource.ApplyResult.FAIL;
		}
		@Override
		protected void onPostExecute(FileSelectorSource.ApplyResult r)
		{
			if(progress_dialog != null)
				progress_dialog.cancel();
			String e = null;
			switch(r)
			{
			case FAIL:					e = getString(R.string.file_select_failed);				break;
			case UNABLE_CONNECT1:		e = getString(R.string.file_select_unable_connect1);	break;
			case UNABLE_CONNECT2:		e = getString(R.string.file_select_unable_connect2);	break;
			case INVALID_INFO:			e = getString(R.string.file_select_invalid_info);		break;
			case NOT_AVAILABLE:			e = getString(R.string.file_select_not_available);		break;
			case UNSUPPORTED_FORMAT:	e = getString(R.string.file_select_unsupported_format);	break;
			}
			if(e != null)
			{
				String me = getString(R.string.file_select_open_error) + e;
				Toast.makeText(getApplicationContext(), me, Toast.LENGTH_LONG).show();
			}
			async_task = false;
			finish();
		}
	}

	private class UpdateAsync extends AsyncTask<Void, Void, FileSelectorSource.GetItemsResult>
	{
		private FileSelector owner;
		private String select_after_update;
		ProgressDialog progress_dialog = null;
		UpdateAsync(FileSelector _owner, final String _select_after_update)
		{
			owner = _owner;
			select_after_update = _select_after_update;
		}
		@Override
		protected void onPreExecute()
		{
			async_task = true;
			if(LongUpdate())
				progress_dialog = ProgressDialog.show(owner, getString(R.string.accessing_web), getString(R.string.gathering_list));
		}
		@Override
		protected FileSelectorSource.GetItemsResult doInBackground(Void... args)
		{
			Items().clear();
			for(FileSelectorSource s : sources)
			{
				FileSelectorSource.GetItemsResult r = s.GetItems(State().current_path, Items());
				if(r != FileSelectorSource.GetItemsResult.OK)
					return r;
			}
			return FileSelectorSource.GetItemsResult.OK;
		}
		@Override
		protected void onPostExecute(FileSelectorSource.GetItemsResult r)
		{
			SetItems();
			if(select_after_update.length() > 0)
			{
				SelectItem(select_after_update);
			}
			if(progress_dialog != null)
				progress_dialog.cancel();
			String e = null;
			switch(r)
			{
			case FAIL:					e = getString(R.string.file_select_failed);				break;
			case UNABLE_CONNECT:		e = getString(R.string.file_select_unable_connect1);	break;
			case INVALID_INFO:			e = getString(R.string.file_select_invalid_info);		break;
			}
			if(e != null)
			{
				String me = getString(R.string.file_select_update_error) + e;
				Toast.makeText(getApplicationContext(), me, Toast.LENGTH_LONG).show();
			}
			async_task = false;
		}
	}
	
	private static class MyListAdapter extends BaseAdapter
	{
		private List<FileSelectorSource.Item> items;
		private Context context;
		private LayoutInflater inf;
		private int dp_5, dp_10;
		MyListAdapter(Context _context, List<FileSelectorSource.Item> _items)
		{
			this.context = _context;
			items = _items;
			inf = LayoutInflater.from(context);
			final float scale = context.getResources().getDisplayMetrics().density;
			dp_5 = (int) (5 * scale + 0.5f);
			dp_10 = (int) (10 * scale + 0.5f);
		}
		@Override
		public int getCount() { return items.size(); }
		@Override
		public Object getItem(int pos) { return items.get(pos); }
		@Override
		public long getItemId(int pos) { return 0; }
		@Override
		public View getView(int position, View convertView, ViewGroup parent)
		{
			if(convertView == null)
			{
				convertView = inf.inflate(R.layout.file_selector_item, null);
			}
			TextView t1 = (TextView)convertView.findViewById(R.id.textLine);
			TextView t2 = (TextView)convertView.findViewById(R.id.textLine2);
			FileSelectorSource.Item item = items.get(position);
			t1.setText(item.name);
			t2.setText(item.desc);
			t2.setVisibility(item.desc == null ? View.GONE : 0);
			int p = item.desc == null ? dp_10 : dp_5;
			convertView.setPadding(dp_10, p, dp_10, p);
			return convertView;
		}
	}
}
