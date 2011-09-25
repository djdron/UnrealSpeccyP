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
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.os.Bundle;
import app.usp.*;

public class FileSelectorFS extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		sources.add(new FSSFS());
    }
	@Override
	protected void onResume()
	{
		if(Items().size() == 0)
		{
			File last_file = new File(Emulator.the.GetLastFile());
			State().last_name = last_file.getName();
			State().current_path = last_file.getParentFile();
			if(State().current_path == null || !State().current_path.exists())
			{
				State().current_path = new File("/");
				State().last_name = "";
			}
		}
		super.onResume();
	}
	class FSSFS extends FileSelectorSource
	{
		public boolean GetItems(final File path, List<Item> items)
		{
			if(path.getParent() != null)
			{
				items.add(new Item("/.."));
			}
			if(path.canRead())
			{
				File[] files = path.listFiles();
				for(File f : files)
				{
					String name = f.getName();
					if(f.isDirectory())
						items.add(new Item("/" + name));
					else if(Emulator.the.FileTypeSupported(name))
						items.add(new Item(name));
				}
				class CmpNames implements Comparator<Item>
				{
					@Override
					public int compare(final Item _a, final Item _b)
					{
						final String a = _a.name;
						final String b = _b.name;
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
				}
				Collections.sort(items, new CmpNames());
			}
			return true;
		}
		public boolean ApplyItem(Item item)
		{
			File f = new File(State().current_path.getPath() + "/" + item.name);
			Emulator.the.Open(f.getAbsolutePath());
			return true;
		}
	}
}
