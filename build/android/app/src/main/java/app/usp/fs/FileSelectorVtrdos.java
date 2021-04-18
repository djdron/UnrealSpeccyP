/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

import org.json.JSONException;
import org.json.JSONObject;

import app.usp.R;

public class FileSelectorVtrdos extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
	@Override
	boolean LongUpdate(final File path) { return PathLevel(path) >= 2 || path.getPath().equals(parser_updates_root); }
	@Override
	int LongUpdateTitle() { return R.string.accessing_web; }
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		sources.add(new ParserGames());
		sources.add(new ParserDemos());
		sources.add(new ParserPress());
		sources.add(new ParserUpdates());
	}
	protected final String base_url = "https://vtrd.in";
	abstract class FSSVtrdosHTML extends FileSelectorSourceHTML
	{
		protected final String VTRDOS_FS = getApplicationContext().getFilesDir().toString() + "/vtrdos";
		@Override
		public String TextEncoding() { return "utf-8"; }
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			File file = new File(VTRDOS_FS + item.url);
			return OpenFile(base_url + item.url, file, progress);
		}
	}
	class ParserGames extends FSSVtrdosHTML
	{
		@Override
		public String FullURL(final String _url) { return base_url + "/games.php?t=" + _url; }
		private final String[] ITEMS = new String[]
			{	"/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B",
				"/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V",
				"/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMSURLS = new String[]
		    {	"full_ver", "demo_ver", "translat", "remix", "123",
				"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
				"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
		    };
		private final String[] PATTERNS = new String[] { "<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td><td>.+</a></td>" };
		@Override
		public final String Root() { return "/games"; }
		@Override
		public final String[] Patterns() { return PATTERNS; }
		@Override
		public void PatternGet(List<Item> items, Matcher m, final String _name)
		{
			Item item = new Item(this, m.group(2));
			item.desc = m.group(3) + " / " + m.group(4) + " [" + m.group(5) + "]";
			item.url = m.group(1);
			items.add(item);
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
	}
	class ParserDemos extends FSSVtrdosHTML
	{
		@Override
		public String FullURL(final String _url)
		{
			if(_url.charAt(0) == '/')
				return base_url + _url + ".php";
			return base_url + "/skin/party.php?year=" + _url;
		}
		private final String[] ITEMS = new String[]
			{	"/Russian", "/Other", "/1995", "/1996", "/1997", "/1998", "/1999", "/2000", "/2001",
				"/2002", "/2003", "/2004", "/2005", "/2006", "/2007", "/2008", "/2009", "/2010",
				"/2011", "/2012", "/2013", "/2014", "/2015", "/2016", "/2017", "/2018", "/2019", "/2020",
				"/2021"
			};
		private final String[] ITEMSURLS = new String[]
			{	"/russian", "/other", "1995", "1996", "1997", "1998", "1999", "2000", "2001",
				"2002", "2003", "2004", "2005", "2006", "2007", "2008", "2009", "2010",
				"2011", "2012", "2013", "2014", "2015", "2016", "2017", "2018", "2019", "2020",
				"2021"
			};
		private final String[] PATTERNS = new String[]
			{   "<a href=\"(.+?)\" target=\"demozdown\">(.+?)</a>",
				"<a href=\"(.+?)\">&nbsp;&nbsp;(.+)</a></td><td>(.+?)</td>",
			};
		@Override
		public final String Root() { return "/demos"; }
		@Override
		public final String[] Patterns() { return PATTERNS; }
		@Override
		public void PatternGet(List<Item> items, Matcher m, final String _name)
		{
			Item item = new Item(this, m.group(2));
			item.url = "/" + m.group(1);
			if(m.groupCount() > 2)
			{
				item.desc = m.group(3);
			}
			items.add(item);
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
		final String DEMO_SIGN = "demo.php?party=";
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			int idx = item.url.indexOf(DEMO_SIGN);
			if(idx < 0)
				return super.ApplyItem(item, progress);
			try
			{
				String s = LoadText(base_url + "/" + DEMO_SIGN + item.url.substring(idx + DEMO_SIGN.length()), TextEncoding(), progress);
				if(s == null)
					return ApplyResult.UNABLE_CONNECT1;
				if(progress.Canceled())
					return ApplyResult.CANCELED;

				Pattern pt = Pattern.compile("<a target=\"_blank\" href=\"(.+?)\">(.+?)</a>");
				Matcher m = pt.matcher(s);
				if(m.find())
				{
					Item item2 = new Item(this, m.group(2));
					if(m.group(1).startsWith("../"))
						item2.url = "/" + m.group(1).substring(3);
					else
						item2.url = "/" + m.group(1);
					return super.ApplyItem(item2, progress);
				}
			}
			catch(Exception e)
			{
			}
			return ApplyResult.FAIL;
		}
	}
	class ParserPress extends FSSVtrdosHTML
	{
        @Override
        public String FullURL(final String _url) { return base_url + "/press.php?l=" + _url; }
		private final String[] ITEMS = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMSURLS = new String[]
			{	"1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1",
				"2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2"
			};
		private final String[] PATTERNS = new String[]
			{	"<td class=\"nowrap\"><b>(.+?)</b></td>\n<td>(?s)(.+?)</td>\n</tr>" };
		@Override
		public final String Root() { return "/press"; }
		@Override
		public final String[] Patterns() { return PATTERNS; }
		@Override
		public void PatternGet(List<Item> items, Matcher m, final String _name)
		{
			String name = m.group(1);
			char ch1 = _name.charAt(1);
			char ch0 = name.charAt(0);
			if(Character.isDigit(ch1))
			{
				if(Character.isLetter(ch0))
					return;
			}
			else if(ch0 != ch1)
				return;

			Pattern pt = Pattern.compile("<span style=\"white-space: nowrap;\"><a href=\"(.+?)\">(.+?)</a>");
			Matcher m2 = pt.matcher(m.group(2));
			while(m2.find())
			{
				Item item = new Item(this, name + " - " + m2.group(2));
				item.url = "/" + m2.group(1);
				items.add(item);
			}
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
	}
	static protected final String parser_updates_root = "/updates";
	class ParserUpdates extends FileSelectorSourceJSON
	{
		protected final String VTRDOS_FS = getApplicationContext().getFilesDir().toString() + "/vtrdos";
		@Override
		public String TextEncoding() { return "utf-8"; }
		@Override
		public String FullURL(final String _url) { return base_url + "/updates.php?json"; }
		@Override
		public final String Root() { return parser_updates_root; }
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			String p = item.url;
			if(!p.startsWith(base_url))
				return ApplyResult.FAIL;
			File file = new File(VTRDOS_FS + p.substring(base_url.length()));
			return OpenFile(p, file, progress);
		}
		@Override
		protected void JsonGet(List<FileSelectorSource.Item> items, JSONObject ji, final String _name)
		{
			String t = ji.optString("title", "");
			if(t.isEmpty())
				return;
			String u = ji.optString("url", "");
			if(u.isEmpty())
				return;
			long size = ji.optLong("filesize", 0);
			if(size > 5*1024*1024)
				return;
			Item item = new Item(this, t);
			item.url = u;
			item.desc = ji.optString("info", "");
			items.add(item);
		}
		private final String[] EMPTY_ITEMS = {};
		@Override
		public final String[] Items() { return EMPTY_ITEMS; }
		@Override
		public final String[] ItemsURLs() { return EMPTY_ITEMS; }
	}
}
