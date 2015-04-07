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

package app.usp.fs;

import java.io.File;
import java.util.Collections;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.List;
import java.util.zip.ZipFile;
import java.util.zip.ZipEntry;

import android.os.Bundle;

import app.usp.Emulator;

public class FileSelectorFS extends FileSelector
{
	private static State state = new State();
	private FSSFS fssfs = new FSSFS();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate(final File path) { return false; } //fssfs.ZipPath(path) != null;
	@Override
	int LongUpdateTitle() { return 0; } //R.string.reading_archive; }
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		sources.add(fssfs);
    }
	@Override
	protected void onResume()
	{
		Items().clear(); // always update list because of changed files on other tabs
		File last_file = new File(Emulator.the.GetLastFile());
		State().last_name = last_file.getName();
		State().current_path = last_file.getParentFile();
		boolean reset_path = State().current_path == null;
		if(!reset_path)
		{
			File zip_path = fssfs.ZipPath(State().current_path);
			reset_path = (zip_path == null && !State().current_path.exists());
		}
		if(reset_path)
		{
			State().current_path = new File("/");
			State().last_name = "";
		}
		super.onResume();
	}
	class FSSFS extends FileSelectorSource
	{
		static final String ZIP_EXT = "zip";
		private boolean IsZipName(final String name)
		{
			return FilenameUtils.getExtension(name).equalsIgnoreCase(ZIP_EXT);
		}
		public final File ZipPath(final File path)
		{
			File zip_path = new File(path.getPath());
			for(;zip_path != null;)
			{
				if(zip_path.canRead() && !zip_path.isDirectory() && IsZipName(zip_path.toString()))
				{
					break;
				}
				zip_path = zip_path.getParentFile();
			}
			return zip_path;
		}
		public GetItemsResult GetItems(final File path, List<Item> items, FileSelectorProgress progress)
		{
			if(path.getParent() != null)
			{
				items.add(new Item("/.."));
			}
			//try open ZIP
			File zip_path = ZipPath(path);
			if(zip_path != null)
			{
				// read items from zip
				try
				{
					ZipFile zif = new ZipFile(zip_path, ZipFile.OPEN_READ);
					for(Enumeration<? extends ZipEntry> e = zif.entries(); e.hasMoreElements();)
					{
						ZipEntry ze = e.nextElement();

//					ZipInputStream zis = new ZipInputStream(new BufferedInputStream(new FileInputStream(zip_path)));
//					ZipEntry ze;
//					while((ze = zis.getNextEntry()) != null)
//					{
						File zef = new File(zip_path.getPath() + "/" + ze.getName());
						File zef_ok = zef.getParentFile();
						if(zef_ok.compareTo(path) == 0)
						{
							File zname = new File(ze.getName());
							if(ze.isDirectory())
								items.add(new Item("/" + zname.getName()));
							else if(Emulator.the.FileTypeSupported(zname.getName()))
								items.add(new Item(zname.getName()));
						}
						if(progress.Canceled())
							return GetItemsResult.CANCELED;
					}
				}
				catch(Exception ex)
				{
				}
			}
			else if(path.canRead())
			{
				File[] files = path.listFiles();
				for(File f : files)
				{
					String name = f.getName();
					if(f.isDirectory() || IsZipName(name))
						items.add(new Item("/" + name));
					else if(Emulator.the.FileTypeSupported(name))
						items.add(new Item(name));
				}
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
			return GetItemsResult.OK;
		}
		public ApplyResult ApplyItem(Item item, FileSelectorProgress progress)
		{
			File f = new File(State().current_path.getPath() + "/" + item.name);
			return Emulator.the.Open(f.getAbsolutePath()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
		}
	}
}
