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
import java.io.IOException;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import android.os.Bundle;
import app.usp.Emulator;
import app.usp.R;

public class FileSelectorWOS extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate(final File path) { return PathLevel(path) >= 2; }
	@Override
	int LongUpdateTitle() { return R.string.accessing_web; }

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		sources.add(new ParserGames());
		sources.add(new ParserAdventures());
		sources.add(new ParserUtilities());
	}
	abstract class FSSWOS extends FSSHtml
	{
		private final String WOS_FS = StoragePath() + "wos";
		public String BaseURL() { return "http://www.worldofspectrum.org"; }
		public String FullURL(final String _url) { return BaseURL() + _url + ".html"; }
		public String HtmlEncoding() { return "iso-8859-1"; }
		private final String[] ITEMS2 = new String[]
				{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
						"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
				};
		private final String[] PATTERNS = new String[] { "<A HREF=\"/infoseekid.cgi\\?id=(\\d+)\">(.+)</A>\\s+(\\d*) (.+?)\\s+\\s+" };
		@Override
		public final String[] Patterns() { return PATTERNS; }
		@Override
		public void Get(List<Item> items, Matcher m, final String url, final String _name)
		{
			Item item = new Item();
			item.name = m.group(2);
			item.desc = m.group(4) + " " + m.group(3);
			item.url = m.group(1);
			items.add(item);
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		final static String HTTP_URL = "http://www.worldofspectrum.org/pub/sinclair";
		public ApplyResult ApplyItem(Item item, FileSelectorProgress progress)
		{
			String s = LoadText("http://www.worldofspectrum.org/infoseekid.cgi?id=" + item.url, HtmlEncoding(), progress);
			if(s == null)
				return ApplyResult.UNABLE_CONNECT1;
			if(progress.Canceled())
				return ApplyResult.CANCELED;

			String url = null;
			Pattern pt = Pattern.compile("<A HREF=\"(.+?)\" TITLE=\"Download to play off-line in an emulator\">.+?</A>");
			Matcher m = pt.matcher(s);
			if(m.find())
			{
				url = BaseURL() + m.group(1);
			}
			if(url == null)
				return ApplyResult.NOT_AVAILABLE;
			if(!url.startsWith(HTTP_URL))
				return ApplyResult.FAIL;
			try
			{
				String p = url.substring(HTTP_URL.length());
				File f = new File(WOS_FS + p).getCanonicalFile();
				if(!LoadFile(url, f, progress))
					return ApplyResult.UNABLE_CONNECT2;
				if(progress.Canceled())
					return ApplyResult.CANCELED;
				return Emulator.the.Open(f.getAbsolutePath()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
			}
			catch(IOException e) { return ApplyResult.FAIL; }
		}
	}
	class ParserGames extends FSSWOS
	{
		private final String[] ITEMS2URLS = new String[]
		    {	"/games/1", "/games/a", "/games/b", "/games/c", "/games/d", "/games/e", "/games/f",
				"/games/g", "/games/h", "/games/i", "/games/j", "/games/k", "/games/l", "/games/m",
				"/games/n", "/games/o", "/games/p", "/games/q", "/games/r", "/games/s", "/games/t",
				"/games/u", "/games/v", "/games/w", "/games/x", "/games/y", "/games/z"
		    };
		@Override
		public final String Root() { return "/games"; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
	class ParserAdventures extends FSSWOS
	{
		private final String[] ITEMS2URLS = new String[]
		    {	"/textadv/1", "/textadv/a", "/textadv/b", "/textadv/c", "/textadv/d", "/textadv/e", "/textadv/f",
				"/textadv/g", "/textadv/h", "/textadv/i", "/textadv/j", "/textadv/k", "/textadv/l", "/textadv/m",
				"/textadv/n", "/textadv/o", "/textadv/p", "/textadv/q", "/textadv/r", "/textadv/s", "/textadv/t",
				"/textadv/u", "/textadv/v", "/textadv/w", "/textadv/x", "/textadv/y", "/textadv/z"
		    };
		@Override
		public final String Root() { return "/adventures"; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
	class ParserUtilities extends FSSWOS
	{
		private final String[] ITEMS2URLS = new String[]
		    {	"/utils/1", "/utils/a", "/utils/b", "/utils/c", "/utils/d", "/utils/e", "/utils/f",
				"/utils/g", "/utils/h", "/utils/i", "/utils/j", "/utils/k", "/utils/l", "/utils/m",
				"/utils/n", "/utils/o", "/utils/p", "/utils/q", "/utils/r", "/utils/s", "/utils/t",
				"/utils/u", "/utils/v", "/utils/w", "/utils/x", "/utils/y", "/utils/z"
		    };
		@Override
		public final String Root() { return "/utilities"; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
}
