package app.usp;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.app.ListActivity;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.view.View;

public class FileSelector extends ListActivity
{
	private List<String> items = new ArrayList<String>();
	private File current_path = new File("/");
	@Override
	protected void onResume()
	{
		super.onResume();
		File last_file = new File(Emulator.the.GetLastFile());
		String last_name = last_file.getName();
		current_path = last_file.getParentFile();
		if(current_path == null || !current_path.exists())
		{
			current_path = new File("/");
			last_name = "";
		}
		Update();
		SelectItem(last_name);
	}
	private void SelectItem(final String name)
	{
		if(name.length() > 0)
		{
			int x = items.indexOf(name);
			if(x >= 0)
				getListView().setSelection(x);
		}
	}
    private void Update()
    {
    	items.clear();
    	if(current_path.getParent() != null)
    	{
    		items.add("/..");
    	}
    	if(current_path.canRead())
    	{
	    	File[] files = current_path.listFiles();
	    	for(File f : files)
	    	{
	    		if(f.isDirectory())
	    			items.add("/" + f.getName());
	    		else
	    			items.add(f.getName());
	    	}
			class CmpNames implements Comparator<String>
			{
				@Override
				public int compare(final String a, final String b)
				{
					if(a.length() == 0 || b.length() == 0)
						return a.compareToIgnoreCase(b);
					final boolean adir = a.charAt(0) == '/';
					final boolean bdir = b.charAt(0) == '/';
					if(adir != bdir)
					{
						return adir ? -1 : +1;
					}
					return a.compareToIgnoreCase(b);
				}
			};
	    	Collections.sort(items, new CmpNames());
    	}
		ArrayAdapter<String> a = new ArrayAdapter<String>(this, R.layout.file_selector_item, items);
		setListAdapter(a);
    }
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		String f = items.get(position);
		if(f.equals("/.."))
		{
			File parent = current_path.getParentFile();
			if(parent != null)
			{
				String name = "/" + current_path.getName();
				current_path = parent;
				Update();
				SelectItem(name);
			}
		}
		else
		{
			if(f.startsWith("/"))
			{
				current_path = new File(current_path.getPath() + f);
				Update();
			}
			else
			{
				current_path = new File(current_path.getPath() + "/" + f);
				Emulator.the.Open(current_path.getAbsolutePath());
				finish();
			}
		}
	}
}
