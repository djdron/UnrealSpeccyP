/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2019 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import android.os.Environment;

import app.usp.Emulator;

public class FileSelectorFS extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate(final File path) { return false; }
	@Override
	int LongUpdateTitle() { return 0; }
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		sources.add(new FileSelectorSourceFS_Cache());
		sources.add(new FileSelectorSourceFS_ExternalStorage());
		sources.add(new FileSelectorSourceFS_ExternalStorageAll());
    }
	@Override
	protected void onResume()
	{
		Items().clear(); // always update list because of changed files on other tabs
		String last_file = Emulator.the.GetLastFile();
		FileSelectorSourceFS fss = null;
		for(FileSelectorSource s : sources)
		{
			FileSelectorSourceFS fs = (FileSelectorSourceFS)s;
			String r = fs.RootPath().getPath();
			if(last_file.startsWith(r))
			{
				File last_file2 = new File(last_file);
				if(last_file2.exists())
				{
					fss = fs;
					break;
				}
				File zip_path = fs.ZipPath(last_file2.getParentFile());
				if(zip_path != null)
				{
					fss = fs;
					break;
				}
			}
		}
		if(fss != null)
		{
			String r = fss.RootPath().getPath();
			File last_file_local = new File(fss.Root() + "/" + last_file.substring(r.length()));
			State().last_name = last_file_local.getName();
			State().current_path = last_file_local.getParentFile();
		}
		else
		{
			State().current_path = new File("/");
			State().last_name = "";
		}
		super.onResume();
	}
	abstract class FileSelectorSourceFS extends FileSelectorSource
	{
		abstract public String Root();
		abstract public File RootPath();
		static final String ZIP_EXT = "zip";
		private boolean IsZipName(final String name)
		{
			return FilenameUtils.getExtension(name).equalsIgnoreCase(ZIP_EXT);
		}
		public final File ZipPath(final File path)
		{
			File zip_path = path;
			while(zip_path != null)
			{
				if(zip_path.canRead() && !zip_path.isDirectory() && IsZipName(zip_path.toString()))
				{
					break;
				}
				zip_path = zip_path.getParentFile();
			}
			return zip_path;
		}
		@Override
		public GetItemsResult GetItems(final File _path, List<Item> items, FileSelector.Progress progress)
		{
			File path_up = _path.getParentFile();
			if(path_up == null)
			{
				items.add(new Item(this, Root()));  // add roots for sources
				return GetItemsResult.OK;
			}
			String p = _path.getPath();
			if(!p.startsWith(Root())) // not our source
				return GetItemsResult.OK;
			items.add(new Item(this, "/.."));

			File path = new File(RootPath().getPath() + "/" + p.substring(Root().length()));

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

						File zef = new File(zip_path.getPath() + "/" + ze.getName());
						File zef_ok = zef.getParentFile();
						if(zef_ok.equals(path))
						{
							File zname = new File(ze.getName());
							if(ze.isDirectory())
								items.add(new Item(this, "/" + zname.getName()));
							else if(Emulator.the.FileTypeSupported(zname.getName()))
								items.add(new Item(this, zname.getName()));
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
						items.add(new Item(this, "/" + name));
					else if(Emulator.the.FileTypeSupported(name))
						items.add(new Item(this, name));
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
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			File f = new File(State().current_path.getPath() + "/" + item.name);
			File n = new File(RootPath().getPath() + "/" + f.getPath().substring(Root().length()));
			return Emulator.the.Open(n.toString()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
		}
	}

	class FileSelectorSourceFS_Cache extends FileSelectorSourceFS
	{
		@Override
		public File RootPath() { return getCacheDir(); }
		@Override
		public String Root() { return "/cache"; }
	}
	class FileSelectorSourceFS_ExternalStorage extends FileSelectorSourceFS
	{
		@Override
		public File RootPath() { return Environment.getExternalStorageDirectory(); }
		@Override
		public String Root() { return "/sdcard"; }
	}
	class FileSelectorSourceFS_ExternalStorageAll extends FileSelectorSourceFS
	{
		@Override
		public File RootPath() { return new File("/storage"); }
		@Override
		public String Root() { return "/storage"; }
	}
}
