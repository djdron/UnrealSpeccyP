/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import java.util.List;
import java.util.regex.Matcher;

import android.os.Bundle;
import app.usp.Emulator;

public class FileSelectorRZX extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate() { return PathLevel(State().current_path) >= 1; }
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		sources.add(new FSSRZX());
	}
	class FSSRZX extends FSSWeb
	{
		private static final String RZX_FS = "/sdcard/usp/rzx";
		public String BaseURL() { return "http://www.rzxarchive.co.uk"; }
		public String HtmlExt() { return ".php"; }
		public String HtmlEncoding() { return "iso-8859-1"; }
		public ApplyResult ApplyItem(Item item)
		{
			try
			{
				String p = item.url;
				File file = new File(RZX_FS + p).getCanonicalFile();
				if(!LoadFile(BaseURL() + p, file))
					return ApplyResult.UNABLE_CONNECT2;
				return Emulator.the.Open(file.getAbsolutePath()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
			}
			catch(Exception e)
			{
			}
			return ApplyResult.FAIL;
		}
		public GetItemsResult GetItems(final File path, List<Item> items)
		{
			File path_up = path.getParentFile();
			if(path_up == null)
			{
				for(String i : Items2())
				{
					items.add(new Item(i));
				}
				return GetItemsResult.OK;
			}
			items.add(new Item("/.."));
			int idx = 0;
			String n = "/" + path.getName().toString();
			for(String i : Items2())
			{
				if(i.equals(n))
				{
					return ParseURL(Items2URLs()[idx], items, n);
				}
				++idx;
			}
			return GetItemsResult.FAIL;
		}
		private final String[] ITEMS2 = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
				"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
				"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
		    {	"/0", "/a", "/b", "/c", "/d", "/e", "/f", "/g", "/h",
				"/i", "/j", "/k", "/l", "/m", "/n", "/o", "/p", "/q",
				"/r", "/s", "/t", "/u", "/v", "/w", "/x", "/y", "/z"
		    };
		private final String[] PATTERNS = new String[] { "<tr><td><font size=2>(.+?)(?:<br>.+?|</td>)<td align=center><font size=2>(.+?)</td><td align=center>(?:<font size=1><A HREF=\"http://www.thunderstats.com/download.cgi\\?http://www.rzxarchive.co.uk(.+?)\"|<font size=2 color=red>).+?" };
		@Override
		public final String Root() { return null; }
		@Override
		public final String[] Patterns() { return PATTERNS; }		
		@Override
		public void Get(List<Item> items, Matcher m, final String url, final String _name)
		{
			if(m.group(3) != null)
			{
				Item item = new Item();
				item.name = m.group(1);
				item.name = item.name.replaceAll("&#39;", "'");
				item.name = item.name.replaceAll("&amp;", "&");
				item.desc = m.group(2);
				item.desc = item.desc.replaceAll("&#237;", "i");
				item.url = m.group(3);
				items.add(item);
			}
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
}
