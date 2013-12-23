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
import java.net.URLConnection;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Environment;

interface FileSelectorProgress
{
	abstract public void OnProgress(Integer current, Integer max);
	abstract public boolean Canceled();
}

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
	enum GetItemsResult { OK, FAIL, UNABLE_CONNECT, INVALID_INFO, CANCELED }
	abstract public GetItemsResult GetItems(final File path, List<Item> items, FileSelectorProgress progress);
	enum ApplyResult { OK, FAIL, UNABLE_CONNECT1, UNABLE_CONNECT2, INVALID_INFO, NOT_AVAILABLE, UNSUPPORTED_FORMAT, CANCELED }
	abstract public ApplyResult ApplyItem(Item item, FileSelectorProgress progress);
}

abstract class FSSWeb extends FileSelectorSource
{
	static final String StoragePath() { return Environment.getExternalStorageDirectory().getPath() + "/usp/"; }
	protected boolean LoadFile(final String _url, final File _name, FileSelectorProgress progress)
	{
		try
		{
			File file = _name.getCanonicalFile();
			File path = file.getParentFile();
			path.mkdirs();
			File file_tmp = new File(file.getPath() + ".tmp");
			FileOutputStream os = new FileOutputStream(file_tmp);
			
			URLConnection connection = new URL(_url).openConnection();
			InputStream is = connection.getInputStream();
			int len = connection.getContentLength();

			byte buffer[] = new byte[256*1024];
			int size = 0;
			int r = -1;
			while((r = is.read(buffer)) != -1)
			{
				os.write(buffer, 0, r);
				size += r;
				if(progress.Canceled())
					break;
				if(len > 0)
					progress.OnProgress(size, len);
			}
			is.close();
			os.close();
			if(!progress.Canceled())
				return file_tmp.renameTo(file);
			else
			{
				file_tmp.delete();
				return true;
			}
		}
		catch(Exception e)
		{
		}
		return false;
	}
	protected String LoadText(final String _url, final String _encoding, FileSelectorProgress progress)
	{
		try
		{
			URLConnection connection = new URL(_url).openConnection();
			InputStream is = connection.getInputStream();
			int len = connection.getContentLength();
			byte buffer[] = new byte[16384];
			ByteBuffer buf = ByteBuffer.allocate(0);
			int r = -1;
			while((r = is.read(buffer)) != -1)
			{
				ByteBuffer buf1 = ByteBuffer.allocate(buf.capacity() + r);
				buf1.put(buf);
				buf1.put(buffer, 0, r);
				buf = buf1;
				buf.rewind();
				if(progress.Canceled())
					return "";
				if(len > 0)
					progress.OnProgress(buf.capacity(), len);
			}
			is.close();
			Charset charset = Charset.forName(_encoding);
			CharsetDecoder decoder = charset.newDecoder();
			return decoder.decode(buf).toString(); 
		}
		catch(Exception e)
		{
		}
		return null;
	}
}

abstract class FSSHtml extends FSSWeb
{
	abstract String Root();
	abstract String BaseURL();
	abstract String FullURL(final String _url);
	abstract String HtmlEncoding();
	abstract String[] Items2();
	abstract String[] Items2URLs();
	abstract String[] Patterns();
	abstract void Get(List<Item> items, Matcher m, final String _url, final String _name);
	public GetItemsResult GetItems(final File path, List<Item> items, FileSelectorProgress progress)
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
				return ParseURL(Items2URLs()[idx], items, n, progress);
			}
			++idx;
		}
		return GetItemsResult.FAIL;
	}
	protected GetItemsResult ParseURL(String _url, List<Item> items, final String _name, FileSelectorProgress progress)
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
