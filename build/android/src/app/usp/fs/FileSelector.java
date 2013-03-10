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
import android.content.DialogInterface;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import android.os.AsyncTask;
import android.os.Bundle;
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
	abstract boolean LongUpdate(final File path);

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
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		getListView().setFastScrollEnabled(true);
	}
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
			new UpdateAsync(this, State().current_path, State().last_name).execute();
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
				new UpdateAsync(this, parent, "/" + State().current_path.getName()).execute();
			}
		}
		else
		{
			if(f.startsWith("/"))
			{
				new UpdateAsync(this, new File(State().current_path.getPath() + f), "").execute();
			}
			else
			{
				State().last_name = f;
				new ApplyAsync(this, Items().get(position)).execute();
			}
		}
	}
	
	static abstract class FSSProgressDialog implements FileSelectorProgress, DialogInterface.OnCancelListener
	{
		FSSProgressDialog(Context _owner, final int _res_title, final int _res_message)
		{
			owner = _owner;
			res_title = _res_title;
			res_message = _res_message;
		}
		void Create()
		{
			pd = CreateProgress();
			pd.show();
			time_last = System.nanoTime();
		}
		void Destroy()
		{
			if(pd != null)
				pd.dismiss();
		}
		void Update(final int value, final int value_max)
		{
			if(pd == null)
				return;
			if(Canceled())
				return;
			final long time = System.nanoTime();
			if(time - time_last < 0.5*1e9) // update each 0.5 sec
				return;
			time_last = time;
			if(pd.isIndeterminate() && (value - value_last)*3 < value_max)
			{
				pd.dismiss();
				pd = CreateProgress();
				pd.setIndeterminate(false);
				pd.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
				pd.show();
			}
			if(!pd.isIndeterminate())
			{
				pd.setMax(value_max);
				pd.setProgress(value);
			}
			value_last = value;
		}
		private ProgressDialog CreateProgress()
		{
			ProgressDialog pd = new ProgressDialog(owner);
			pd.setTitle(owner.getString(res_title));
			pd.setMessage(owner.getString(res_message));
			pd.setOnCancelListener(this);
			pd.setCancelable(true);
			pd.setCanceledOnTouchOutside(false);
			return pd;
		}
		@Override
		public boolean Canceled() { return canceled; }
		@Override
		public void onCancel(DialogInterface di)
		{
			canceled = true;
			pd.dismiss();
			pd = new ProgressDialog(owner);
			pd.setTitle(owner.getString(res_title));
			pd.setMessage(owner.getString(R.string.canceling));
			pd.setCancelable(false);
			pd.show();
		}
		private Context owner;
		private final int res_title;
		private final int res_message;
		private ProgressDialog pd = null;
		private long time_last = 0;
		int value_last = 0;
		boolean canceled = false;
	}

	private class ApplyAsync extends AsyncTask<Void, Integer, FileSelectorSource.ApplyResult>
	{
		private FileSelector owner;
		FileSelectorSource.Item item;
		private FSSProgressDialog progress;
		ApplyAsync(FileSelector _owner, FileSelectorSource.Item _item)
		{
			owner = _owner;
			item = _item;
			progress = new FSSProgressDialog(owner, R.string.accessing_web, R.string.downloading_image)
			{
				@Override
				public void OnProgress(Integer current, Integer max) { publishProgress(current, max); }
			};
		}
		@Override
		protected void onPreExecute()
		{
			async_task = true;
			if(LongUpdate(State().current_path))
				progress.Create();
		}
		@Override
		protected FileSelectorSource.ApplyResult doInBackground(Void... args)
		{
			for(FileSelectorSource s : sources)
			{
				return s.ApplyItem(item, progress);
			}
			return FileSelectorSource.ApplyResult.FAIL;
		}
		@Override
		protected void onProgressUpdate(Integer... values)
		{
			progress.Update(values[0], values[1]);
		}
		@Override
		protected void onPostExecute(FileSelectorSource.ApplyResult r)
		{
			progress.Destroy();
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
			if(r == FileSelectorSource.ApplyResult.OK)
				finish();
		}
	}

	private class UpdateAsync extends AsyncTask<Void, Integer, FileSelectorSource.GetItemsResult>
	{
		private FileSelector owner;
		private final File path;
		private String select_after_update;
		private FSSProgressDialog progress;
		private List<FileSelectorSource.Item> items = new ArrayList<FileSelectorSource.Item>();
		UpdateAsync(FileSelector _owner, final File _path, final String _select_after_update)
		{
			owner = _owner;
			path = _path;
			select_after_update = _select_after_update;
			progress = new FSSProgressDialog(owner, R.string.accessing_web, R.string.gathering_list)
			{
				@Override
				public void OnProgress(Integer current, Integer max) { publishProgress(current, max); }
			};
		}
		@Override
		protected void onPreExecute()
		{
			async_task = true;
			if(LongUpdate(path))
				progress.Create();
		}
		@Override
		protected FileSelectorSource.GetItemsResult doInBackground(Void... args)
		{
			for(FileSelectorSource s : sources)
			{
				FileSelectorSource.GetItemsResult r = s.GetItems(path, items, progress);
				if(r != FileSelectorSource.GetItemsResult.OK)
					return r;
			}
			return FileSelectorSource.GetItemsResult.OK;
		}
		@Override
		protected void onProgressUpdate(Integer... values)
		{
			progress.Update(values[0], values[1]);
		}
		@Override
		protected void onPostExecute(FileSelectorSource.GetItemsResult r)
		{
			progress.Destroy();
			String e = null;
			switch(r)
			{
			case FAIL:					e = getString(R.string.file_select_failed);				break;
			case UNABLE_CONNECT:		e = getString(R.string.file_select_unable_connect1);	break;
			case INVALID_INFO:			e = getString(R.string.file_select_invalid_info);		break;
			case OK:
				State().current_path = path;
				State().items = items;
				SetItems();
				if(select_after_update.length() > 0)
				{
					SelectItem(select_after_update);
				}
				break;
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
