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
import android.content.Context;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
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

	protected List<FileSelectorSource> sources = new ArrayList<FileSelectorSource>();
	@Override
	protected void onResume()
	{
		super.onResume();
		if(Items().size() > 0)
			setListAdapter(new MyListAdapter(this, Items()));
		else
			Update();
		SelectItem(State().last_name);
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
	private void Update()
	{
		Items().clear();
		for(FileSelectorSource s : sources)
		{
			if(s.GetItems(State().current_path, Items()))
				break;
		}
		setListAdapter(new MyListAdapter(this, Items()));
	}
	
	@Override
	protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		String f = Items().get(position).name;
		if(f.equals("/.."))
		{
			File parent = State().current_path.getParentFile();
			if(parent != null)
			{
				String name = "/" + State().current_path.getName();
				State().current_path = parent;
				Update();
				SelectItem(name);
			}
		}
		else
		{
			if(f.startsWith("/"))
			{
				State().current_path = new File(State().current_path.getPath() + f);
				Update();
			}
			else
			{
				State().last_name = f;
				FileSelectorSource.Item item = Items().get(position);
				for(FileSelectorSource s : sources)
				{
					s.ApplyItem(item);
					break;
				}
				finish();
			}
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
