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

import org.json.JSONObject;

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
//		sources.add(new ParserSimulators());
		sources.add(new ParserEducational());
		sources.add(new ParserUtilities());
		sources.add(new ParserDemos());
//		sources.add(new ParserMisc());
//		sources.add(new ParserXRated());
	}
	abstract class FSSWOS extends FileSelectorSourceJSON
	{
		private final String WOS_FS = getApplicationContext().getFilesDir().toString() + "/wos";
		private final String base_url = "https://worldofspectrum.org";
		abstract protected String URLSection();
		@Override
		public String FullURL(final String _url)
		{
			return base_url + "/software/ajax_software_items/" + URLSection() +
					"?columns%5B0%5D%5Bdata%5D=az&columns%5B0%5D%5Bsearch%5D%5Bvalue%5D=" + _url +
					"&columns%5B1%5D%5Bdata%5D=title&columns%5B8%5D%5Bdata%5D=availability&columns%5B8%5D%5Bsearch%5D%5Bvalue%5D=Available" +
					"&order%5B0%5D%5Bcolumn%5D=1&order%5B0%5D%5Bdir%5D=asc&length=-1";
		}
		@Override
		public String TextEncoding() { return "iso-8859-1"; }
		private final String[] ITEMS = new String[]
				{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
					"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
				};
		private final String[] ITEMSURLS = new String[]
				{	"%23", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
					"M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
				};
		@Override
		public GetItemsResult ParseText(final String _text, List<Item> items, final String _name, FileSelector.Progress progress)
		{
			int idx_beg = _text.indexOf("\"data\":");
			int idx_end = _text.lastIndexOf(",\"draw\":");
			if(idx_beg < 0 || idx_end < 0)
				return GetItemsResult.INVALID_INFO;
			idx_beg += 7; // strlen of "data:"
			if(idx_end <= idx_beg)
				return GetItemsResult.INVALID_INFO;
			return super.ParseText(_text.substring(idx_beg, idx_end), items, _name, progress);
		}
		@Override
		protected void JsonGet(List<FileSelectorSource.Item> items, JSONObject ji, final String _name)
		{
			Object to = ji.opt("title");
			if(JSONObject.NULL.equals(to))
				return;
			Object uo = ji.optString("slug");
			if(JSONObject.NULL.equals(uo))
				return;

			String t = to.toString();
			Item item = new Item(this, t);
			item.url = uo.toString();
			item.desc = "";
			String tp = ji.optString("title_publisher");
			if(!tp.isEmpty())
			{
				int idx = tp.indexOf(t);
				if(idx == 0)
				{
					tp = tp.substring(idx + t.length());
					idx = tp.indexOf(" (");
					if(idx == 0 && tp.charAt(tp.length() - 1) == ')')
					{
						tp = tp.substring(2, tp.length() - 1);
						if(!tp.isEmpty())
						{
							item.desc += tp;
						}
					}
				}
			}
			Object yo = ji.opt("release_year");
			if(!JSONObject.NULL.equals(yo))
				item.desc += "'" + yo;
			Object eto = ji.optString("entry_type");
			if(!JSONObject.NULL.equals(eto))
			{
				if(!item.desc.isEmpty())
					item.desc += " - ";
				item.desc += eto;
			}
			items.add(item);
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
		final String URL_START = "/pub/sinclair";
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			String s = LoadText(base_url + "/archive/software/" + URLSection() + "/" + item.url, TextEncoding(), progress);
			if(s == null)
				return ApplyResult.UNABLE_CONNECT1;
			if(progress.Canceled())
				return ApplyResult.CANCELED;

			String url = null;
			Pattern pt = Pattern.compile("<th>Play</th>(?s).+?<th>File</th>.+?<a href=\"(.+?)\"");
			Matcher m = pt.matcher(s);
			if(m.find())
			{
				url = m.group(1);
			}
			if(url == null)
				return ApplyResult.NOT_AVAILABLE;
			int idx = url.indexOf(URL_START);
			if(idx == -1)
				return ApplyResult.FAIL;
			String p = url.substring(idx + URL_START.length());
			File f = new File(WOS_FS + p);
			return OpenFile(url, f, progress);
		}
	}
	class ParserGames extends FSSWOS
	{
		@Override
		protected String URLSection() { return "games"; }
		@Override
		public final String Root() { return "/games"; }
	}
	class ParserAdventures extends FSSWOS
	{
		@Override
		protected String URLSection() { return "text-adventures"; }
		@Override
		public final String Root() { return "/adventures"; }
	}
	class ParserSimulators extends FSSWOS
	{
		@Override
		protected String URLSection() { return "simulators"; }
		@Override
		public final String Root() { return "/simulators"; }
	}
	class ParserEducational extends FSSWOS
	{
		@Override
		protected String URLSection() { return "educational"; }
		@Override
		public final String Root() { return "/educational"; }
	}
	class ParserUtilities extends FSSWOS
	{
		@Override
		protected String URLSection() { return "utilities"; }
		@Override
		public final String Root() { return "/utilities"; }
	}
	class ParserDemos extends FSSWOS
	{
		@Override
		protected String URLSection() { return "demos"; }
		@Override
		public final String Root() { return "/demos"; }
	}
	class ParserMisc extends FSSWOS
	{
		@Override
		protected String URLSection() { return "miscellaneous"; }
		@Override
		public final String Root() { return "/misc"; }
	}
	class ParserXRated extends FSSWOS
	{
		@Override
		protected String URLSection() { return "xrated"; }
		@Override
		public final String Root() { return "/xrated"; }
	}
}
