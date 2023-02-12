/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2022 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import java.net.URLConnection;
import java.util.List;

import android.os.Bundle;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONTokener;

import app.usp.R;

public class FileSelectorRZX extends FileSelector
{
	private static final State state = new State();
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
	class FSSRZX extends FileSelectorSourceJSON
	{
		private final String RZX_FS = getApplicationContext().getFilesDir().toString() + "/rzx";
		protected final String base_url = "https://api.vtrd.in/v1/rzx";
		@Override
		public String FullURL(final String _url) { return base_url + _url; }
		@Override
		public String TextEncoding() { return "iso-8859-1"; }
		@Override
		protected void SetupConnection(URLConnection connection)
		{
			connection.setRequestProperty("Authorization", "Bearer SD0XHmQZzbZpK1PyPbIgfL2P4iaCeQ02gTJogfPH43A88FN6B02dFJbJo0Ho");
		}
		@Override
		public ApplyResult ApplyItem(Item item, FileSelector.Progress progress)
		{
			int pos = item.url.lastIndexOf("/rzx/");
			if(pos == -1)
				return ApplyResult.FAIL;
			File file = new File(RZX_FS + item.url.substring(pos + 4));
			return OpenFile(item.url, file, progress);
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
		@Override
		public final String Root() { return null; }

		@Override
		protected JSONArray TextToJSONArray(final String _text)
		{
			try
			{
				JSONObject json = (JSONObject) new JSONTokener(_text).nextValue();
				return json.optJSONArray("data");
			}
			catch(Exception e) { return null; }
		}

		@Override
		public void JsonGet(List<FileSelectorSource.Item> items, JSONObject ji, final String _name)
		{
			String name = ji.optString("name", "");
			if(name.isEmpty())
				return;
			String url = ji.optString("path", "");
			if(url.isEmpty())
				return;
			String info = ji.optString("info", "");
			String author = ji.optString("submitter", "");
			Item item = new Item(this, name);
			item.url = url;
			item.desc = author;
			if(!info.isEmpty())
				item.desc += " / " + info;
			items.add(item);
		}
		@Override
		public final String[] Items() { return ITEMS; }
		@Override
		public final String[] ItemsURLs() { return ITEMSURLS; }
	}
}
