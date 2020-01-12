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
import java.util.List;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import android.os.Bundle;
import app.usp.Emulator;
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
	class FSSBBB extends FileSelectorSourceWEB
	{
		private final String BBB_FS = getApplicationContext().getCacheDir().toString() + "/bbb";
		public String BaseURL() { return "https://bbb.retroscene.org"; }
		public String JsonEncoding() { return "iso-8859-1"; }
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			String p = item.url;
			if(!p.startsWith(BaseURL()))
				return ApplyResult.FAIL;
			File file = new File(BBB_FS + p.substring(BaseURL().length()));
			return OpenFile(p, file, progress);
		}
		public GetItemsResult GetItems(final File path, List<Item> items, FileSelector.Progress progress)
		{
			File path_up = path.getParentFile();
			if(path_up == null)
			{
				for(String i : ITEMS2)
				{
					items.add(new Item(this, i));
				}
				return GetItemsResult.OK;
			}
			items.add(new Item(this, "/.."));
			int idx = 0;
			String n = "/" + path.getName();
			for(String i : ITEMS2)
			{
				if(i.equals(n))
				{
					return ParseJSON(ITEMS2URLS[idx], items, n, progress);
				}
				++idx;
			}
			return GetItemsResult.FAIL;
		}
		protected GetItemsResult ParseJSON(String _url, List<Item> items, final String _name, FileSelector.Progress progress)
		{
			String s0 = LoadText(BaseURL() + "/unreal_demos.php?l=" + _url, JsonEncoding(), progress);
			if(s0 == null)
				return GetItemsResult.UNABLE_CONNECT;
			if(progress.Canceled())
				return GetItemsResult.CANCELED;
			String s = "{ \"items\": [ " + s0 + " ] }";
			JSONObject json = null;
			try
			{
				json = new JSONObject(s);
			}
			catch(JSONException e) { return GetItemsResult.INVALID_INFO; }
			JSONArray jitems = null;
			try
			{
				jitems = json.getJSONArray("items");
			}
			catch(JSONException e) { return GetItemsResult.INVALID_INFO; }
			for(int i = 0; i < jitems.length(); ++i)
			{
				try
				{
					JSONObject ji = jitems.getJSONObject(i);
					String t = ji.getString("title");
					if(t == null)
						continue;
					String u = ji.getString("url");
					if(u == null)
						continue;
					String a = ji.getString("author");
					String y = ji.getString("year");
					String c = ji.getString("city");
					Item item = new Item(this, t);
					item.desc = "";
					if(a != null && a.length() > 0)
						item.desc += a;
					if(y != null && y.length() > 0)
						item.desc += "'" + y;
					if(c != null && c.length() > 0)
						item.desc += " / " + c;
					item.url = u;
					items.add(item);
				}
				catch(JSONException e) {}
			}
			return GetItemsResult.OK;
		}
		private final String[] ITEMS2 = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
				"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
				"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
		    {	"@", "a", "b", "c", "d", "e", "f", "g", "h",
				"i", "j", "k", "l", "m", "n", "o", "p", "q",
				"r", "s", "t", "u", "v", "w", "x", "y", "z"
		    };
	}
}
