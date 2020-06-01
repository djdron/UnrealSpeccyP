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

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.util.List;

abstract class FileSelectorSourceJSON extends FileSelectorSourceWEB
{
	abstract protected void JsonGet(List<Item> items, JSONObject j, final String _name);
	@Override
	public GetItemsResult ParseText(final String _text, List<Item> items, final String _name, FileSelector.Progress progress)
	{
		JSONArray jitems = null;
		try
		{
			jitems = (JSONArray) new JSONTokener(_text).nextValue();
		}
		catch(Exception e) { return GetItemsResult.INVALID_INFO; }
		for(int i = 0; i < jitems.length(); ++i)
		{
			try
			{
				JsonGet(items, jitems.getJSONObject(i), _name);
			}
			catch(JSONException e) {}
		}
		return GetItemsResult.OK;
	}
}
