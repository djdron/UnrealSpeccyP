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
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

abstract class FileSelectorSource
{
	class Item
	{
		Item() {}
		Item(final String _name) { name = _name; }
		String name;
		String desc;
		String url;
	};
	enum GetItemsResult { OK, FAIL, UNABLE_CONNECT, INVALID_INFO }
	abstract public GetItemsResult GetItems(final File path, List<Item> items);
	enum ApplyResult { OK, FAIL, UNABLE_CONNECT1, UNABLE_CONNECT2, INVALID_INFO, NOT_AVAILABLE, UNSUPPORTED_FORMAT }
	abstract public ApplyResult ApplyItem(Item item);
}

abstract class FSSWeb extends FileSelectorSource
{
	abstract String Root();
	abstract String BaseURL();
	abstract String HtmlExt();
	abstract String HtmlEncoding();
	abstract String[] Items2();
	abstract String[] Items2URLs();
	abstract String[] Patterns();
	abstract void Get(List<Item> items, Matcher m, final String _url, final String _name);
	public GetItemsResult GetItems(final File path, List<Item> items)
	{
		File path_up = path.getParentFile();
		if(path_up == null)
		{
			items.add(new Item(Root()));
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
		items.add(new Item("/.."));
		if(path_up.getParent() == null)
		{
			for(String i : Items2())
			{
				items.add(new Item(i));
			}
			return GetItemsResult.OK;
		}
		int idx = 0;
		String n = "/" + path.getName().toString();
		for(String i : Items2())
		{
			if(i.equals(n))
			{
				return ParseURL(Items2URLs()[idx], items, n);
			}
			++idx;
		}
		return GetItemsResult.FAIL;
	}
	protected boolean LoadFile(final String _url, final File _name)
	{
		try
		{
			File file = _name.getCanonicalFile();
			File path = file.getParentFile();
			path.mkdirs();
			FileOutputStream os = new FileOutputStream(file);
			URL url = new URL(_url);
			InputStream is = url.openStream();
			byte buffer[] = new byte[16384];
			int r = -1;
			while((r = is.read(buffer)) != -1)
			{
				os.write(buffer, 0, r);
			}
			is.close();
			os.close();
			return true;
		}
		catch(Exception e)
		{
		}
		return false;
	}
	protected String LoadText(final String _url, final String _encoding)
	{
		try
		{
			Charset charset = Charset.forName(_encoding);
			CharsetDecoder decoder = charset.newDecoder();
			InputStream is = new URL(_url).openStream();
			byte buffer[] = new byte[16384];
			String s = "";
			int r = -1;
			while((r = is.read(buffer)) != -1)
			{
				CharBuffer cb = decoder.decode(ByteBuffer.wrap(buffer, 0, r));
				s += cb;
			}
			is.close();
			return s;
		}
		catch(Exception e)
		{
		}
		return null;
	}
	private GetItemsResult ParseURL(String _url, List<Item> items, final String _name)
	{
		String s = LoadText(BaseURL() + _url + HtmlExt(), HtmlEncoding());
		if(s == null)
			return GetItemsResult.UNABLE_CONNECT;
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
