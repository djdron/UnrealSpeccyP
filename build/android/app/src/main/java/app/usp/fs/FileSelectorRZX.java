/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2020 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import java.util.regex.Pattern;

import android.os.Bundle;
import app.usp.Emulator;
import app.usp.R;

public class FileSelectorRZX extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate(final File path) { return PathLevel(path) >= 1; }
	@Override
	int LongUpdateTitle() { return R.string.accessing_web; }
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		sources.add(new FSSRZX());
	}
	class FSSRZX extends FileSelectorSourceHTML
	{
		private final String RZX_FS = getApplicationContext().getFilesDir().toString() + "/rzx";
		protected final String base_url = "https://www.rzxarchive.co.uk";
		@Override
		public String FullURL(final String _url) { return base_url + _url + ".php"; }
		@Override
		public String TextEncoding() { return "iso-8859-1"; }
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			File file = new File(RZX_FS + item.url);
			return OpenFile(base_url + item.url, file, progress);
		}
		private final String[] ITEMS = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
				"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
				"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMSURLS = new String[]
		    {	"/0", "/a", "/b", "/c", "/d", "/e", "/f", "/g", "/h",
				"/i", "/j", "/k", "/l", "/m", "/n", "/o", "/p", "/q",
				"/r", "/s", "/t", "/u", "/v", "/w", "/x", "/y", "/z"
		    };
		private final String[] PATTERNS = new String[] { "<font size=2>(.+?)(?:<br>.+?|</td>)<td align=center><font size=2>(.+?)</td><td align=center>(?:<font size=1><A HREF=\"(.+?)\"|<font size=2 color=red>).+?" };
		@Override
		public final String Root() { return null; }
		@Override
		public final String[] Patterns() { return PATTERNS; }
		
		private final String UnescapeHTML(String s)
		{
			String r = s;
			Pattern p = Pattern.compile("&#([0-9]+);");
			Matcher m = p.matcher(r);
			int x = 0;
			while(m.find())
			{
				String c = new String(Character.toChars(Integer.parseInt(m.group(1))));
				r = r.substring(0,  m.start() - x) + c + r.substring(m.end() - x);
				x += m.end() - m.start() - c.length();
			}
			r = r.replaceAll("&amp;", "&");
			return r;
		}
		
		@Override
		public void PatternGet(List<Item> items, Matcher m, final String _name)
		{
			if(m.group(3) != null)
			{
				Item item = new Item(this, UnescapeHTML(m.group(1)));
				item.desc = UnescapeHTML(m.group(2));
				item.url = m.group(3);
				items.add(item);
			}
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
	}
}
