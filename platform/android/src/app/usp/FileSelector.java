package app.usp;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
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
		current_path = new File(Emulator.the.GetLastFolder());
		if(current_path == null || !current_path.exists())
			current_path = new File("/");
		Update();
	}
    private void Update()
    {
    	items.clear();
    	if(current_path.getParent() != null)
    	{
    		items.add("..");
    	}
    	if(current_path.canRead())
    	{
	    	File[] files = current_path.listFiles();
	    	List<String> list = new ArrayList<String>();
	    	for(File f : files)
	    	{
	    		if(f.isDirectory())
	    			list.add("/" + f.getName());
	    	}
	    	Collections.sort(list);
	    	items.addAll(list);
	    	list.clear();
	    	for(File f : files)
	    	{
	    		if(!f.isDirectory())
	    			items.add(f.getName());
	    	}
	    	Collections.sort(list);
	    	items.addAll(list);
    	}
		ArrayAdapter<String> a = new ArrayAdapter<String>(this, R.layout.file_selector_item, items);
		this.setListAdapter(a);
    }
    @Override
    protected void onListItemClick(ListView l, View v, int position, long id)
	{
		super.onListItemClick(l, v, position, id);
		String f = items.get(position);
		if(f.equals(".."))
		{
			File parent = current_path.getParentFile();
			if(parent != null)
				current_path = parent;
		}
		else
		{
			if(f.startsWith("/"))
				current_path = new File(current_path.getPath() + f);
			else
				current_path = new File(current_path.getPath() + "/" + f);
		}
		if(current_path.isFile())
		{
			Emulator.the.Open(current_path.getAbsolutePath());
			finish();
		}
		else
		{
			Update();
		}
	}
}
