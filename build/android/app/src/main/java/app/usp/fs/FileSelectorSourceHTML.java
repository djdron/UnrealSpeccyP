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
import java.util.regex.Matcher;
import java.util.regex.Pattern;


abstract class FileSelectorSourceHTML extends FileSelectorSourceWEB
{
	abstract String Root();
	abstract String BaseURL();
	abstract String FullURL(final String _url);
	abstract String HtmlEncoding();
	abstract String[] Items2();
	abstract String[] Items2URLs();
	abstract String[] Patterns();
	abstract void Get(List<Item> items, Matcher m, final String _url, final String _name);
	@Override
	public GetItemsResult GetItems(final File path, List<Item> items, FileSelector.Progress progress)
	{
		File path_up = path.getParentFile();
		if(path_up == null)
		{
			items.add(new Item(this, Root()));
			return GetItemsResult.OK;
		}
		File r = path;
		for(;;)
		{
			File p = r.getParentFile();
			if(p.getParentFile() == null)
				break;
			r = p;
		}
		if(!r.toString().equals(Root()))
			return GetItemsResult.OK;
		items.add(new Item(this, "/.."));
		if(path_up.getParent() == null)
		{
			for(String i : Items2())
			{
				items.add(new Item(this, i));
			}
			return GetItemsResult.OK;
		}
		int idx = 0;
		String n = "/" + path.getName();
		for(String i : Items2())
		{
			if(i.equals(n))
			{
				return ParseURL(Items2URLs()[idx], items, n, progress);
			}
			++idx;
		}
		return GetItemsResult.FAIL;
	}
	protected GetItemsResult ParseURL(String _url, List<Item> items, final String _name, FileSelector.Progress progress)
	{
		String s = LoadText(FullURL(_url), HtmlEncoding(), progress);
		if(s == null)
			return GetItemsResult.UNABLE_CONNECT;
		if(progress.Canceled())
			return GetItemsResult.CANCELED;
		boolean ok = false;
		for(String p : Patterns())
		{
			Pattern pt = Pattern.compile(p);
			Matcher m = pt.matcher(s);
			while(m.find())
			{
				ok = true;
				Get(items, m, _url, _name);
			}
		}
		if(ok)
			return GetItemsResult.OK;
		return GetItemsResult.INVALID_INFO;
	}
}
