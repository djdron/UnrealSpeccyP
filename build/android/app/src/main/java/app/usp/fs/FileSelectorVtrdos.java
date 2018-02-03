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
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Bundle;
import app.usp.Emulator;
import app.usp.R;

public class FileSelectorVtrdos extends FileSelector
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
		sources.add(new ParserDemos());
		sources.add(new ParserPress());
	}
	abstract class FSSVtrdos extends FSSHtml
	{
		protected final String VTRDOS_FS = StoragePath() + "vtrdos";
		public String BaseURL() { return "https://vtrd.in"; }
		public String FullURL(final String _url) { return BaseURL() + _url + ".htm"; }
        public String HtmlEncoding() { return "utf-8"; }
		public ApplyResult ApplyItem(Item item, FileSelectorProgress progress) { return ApplyResult.TRY_OTHER_SOURCE; }
		public ApplyResult ApplyItemBase(Item item, FileSelectorProgress progress)
		{
			try
			{
				String p = item.url;
				File file = new File(VTRDOS_FS + p).getCanonicalFile();
				if(!LoadFile(BaseURL() + p, file, progress))
					return ApplyResult.UNABLE_CONNECT2;
				if(progress.Canceled())
					return ApplyResult.CANCELED;
				return Emulator.the.Open(file.getAbsolutePath()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
			}
			catch(Exception e)
			{
			}
			return ApplyResult.FAIL;
		}
	}
	class ParserGames extends FSSVtrdos
	{
		@Override
		public String FullURL(final String _url) { return BaseURL() + "/games.php?t=" + _url; }
		private final String[] ITEMS2 = new String[]
			{	"/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B",
				"/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V",
				"/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
		    {	"full_ver", "demo_ver", "translat", "remix", "123",
				"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
				"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
		    };
		private final String[] PATTERNS = new String[] { "<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td>" };
		@Override
		public final String Root() { return "/games"; }
		@Override
		public final String[] Patterns() { return PATTERNS; }		
		@Override
		public void Get(List<Item> items, Matcher m, final String url, final String _name)
		{
			Item item = new Item();
			item.name = m.group(2);
			item.desc = m.group(3) + " / " + m.group(4) + " [" + m.group(5) + "]";
			item.url = m.group(1);
			items.add(item);
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
	class ParserDemos extends FSSVtrdos
	{
		@Override
		public String FullURL(final String _url)
		{
			if(_url.charAt(0) == '/')
				return BaseURL() + _url + ".php";
			return BaseURL() + "/skin/party.php?year=" + _url;
		}
		private final String[] ITEMS2 = new String[]
			{	"/Russian", "/Other", "/1995", "/1996", "/1997", "/1998", "/1999", "/2000", "/2001",
				"/2002", "/2003", "/2004", "/2005", "/2006", "/2007", "/2008", "/2009", "/2010",
				"/2011", "/2012", "/2013", "/2014", "/2015"
			};
		private final String[] ITEMS2URLS = new String[]
			{	"/russian", "/other", "1995", "1996", "1997", "1998", "1999", "2000", "2001",
				"2002", "2003", "2004", "2005", "2006", "2007", "2008", "2009", "2010",
				"2011", "2012", "2013", "2014", "2015"
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
		public void Get(List<Item> items, Matcher m, final String url, final String _name)
		{
			Item item = new Item();
			item.name = m.group(2);
			item.url = "/" + m.group(1);
			if(m.groupCount() > 2)
			{
				item.desc = m.group(3);
			}
			items.add(item);
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
		final String DEMO_SIGN = "demo.php?party=";
		@Override
		public ApplyResult ApplyItem(Item item, FileSelectorProgress progress)
		{
			int idx = item.url.indexOf(DEMO_SIGN);
			if(idx < 0)
				return ApplyItemBase(item, progress);
			try
			{
				String s = LoadText(BaseURL() + "/" + DEMO_SIGN + item.url.substring(idx + DEMO_SIGN.length()), HtmlEncoding(), progress);
				if(s == null)
					return ApplyResult.UNABLE_CONNECT1;
				if(progress.Canceled())
					return ApplyResult.CANCELED;

				Pattern pt = Pattern.compile("<a target=\"_blank\" href=\"(.+?)\">(.+?)</a>");
				Matcher m = pt.matcher(s);
				if(m.find())
				{
					Item item2 = new Item();
					item2.name = m.group(2);
					if(m.group(1).startsWith("../"))
						item2.url = "/" + m.group(1).substring(3);
					else
						item2.url = "/" + m.group(1);
					return ApplyItemBase(item2, progress);
				}
			}
			catch(Exception e)
			{
			}
			return ApplyResult.FAIL;
		}
	}
	class ParserPress extends FSSVtrdos
	{
        @Override
        public String FullURL(final String _url) { return BaseURL() + "/press.php?l=" + _url; }
		private final String[] ITEMS2 = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
			{	"1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1",
				"2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2"
			};
		private final String[] PATTERNS = new String[]
			{	"<td class=\"nowrap\"><b>(.+?)</b></td>\n<td>(?s)(.+?)\n</td>\n</tr>" };
		@Override
		public final String Root() { return "/press"; }
		@Override
		public final String[] Patterns() { return PATTERNS; }
		@Override
		public void Get(List<Item> items, Matcher m, final String url, final String _name)
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

			Pattern pt = Pattern.compile("<a class=\"rpad\" href=\"(.+?)\">(.+?)</a>");
			Matcher m2 = pt.matcher(m.group(2));
			while(m2.find())
			{
				Item item = new Item();
				item.name = name + " - " + m2.group(2);
				item.url = "/" + m2.group(1);
				items.add(item);
			}
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
}
