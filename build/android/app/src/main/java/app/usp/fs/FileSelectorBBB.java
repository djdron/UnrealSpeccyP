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

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Bundle;
import app.usp.R;

public class FileSelectorBBB extends FileSelector
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
		sources.add(new FSSBBB());
	}
	class FSSBBB extends FileSelectorSourceJSON
	{
		private final String BBB_FS = getApplicationContext().getFilesDir().toString() + "/bbb";
		public String base_url = "https://bbb.retroscene.org";
		@Override
		public String FullURL(final String _url) { return base_url + "/unreal_demos.php?l=" + _url; }
		@Override
		public String TextEncoding() { return "iso-8859-1"; }
		@Override
		public final String Root() { return null; }
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			String p = item.url;
			if(!p.startsWith(base_url))
				return ApplyResult.FAIL;
			File file = new File(BBB_FS + p.substring(base_url.length()));
			return OpenFile(p, file, progress);
		}
		@Override
		public GetItemsResult ParseText(final String _text, List<Item> items, final String _name, FileSelector.Progress progress)
		{
			return super.ParseText("[" + _text + "]", items, _name, progress);
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
			String a = ji.optString("author", "");
			String y = ji.optString("year", "");
			String c = ji.optString("city", "");
			Item item = new Item(this, t);
			item.url = u;
			item.desc = a;
			if(!y.isEmpty())
				item.desc += "'" + y;
			if(!c.isEmpty())
				item.desc += " / " + c;
			items.add(item);
		}
		private final String[] ITEMS = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
				"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
				"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMSURLS = new String[]
		    {	"@", "a", "b", "c", "d", "e", "f", "g", "h",
				"i", "j", "k", "l", "m", "n", "o", "p", "q",
				"r", "s", "t", "u", "v", "w", "x", "y", "z"
		    };
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
	}
}
