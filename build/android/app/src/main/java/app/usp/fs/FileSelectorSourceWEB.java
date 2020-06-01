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
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.util.List;

import app.usp.Emulator;


abstract class FileSelectorSourceWEB extends FileSelectorSource
{
	abstract String Root();
	abstract String FullURL(final String _url);
	abstract String TextEncoding();
	abstract String[] Items();
	abstract String[] ItemsURLs();

	@Override
	public GetItemsResult GetItems(final File path, List<Item> items, FileSelector.Progress progress)
	{
		File path_up = path.getParentFile();
		if(Root() == null)
		{
			if(path_up == null)
			{
				for(String i : Items())
				{
					items.add(new Item(this, i));
				}
				return GetItemsResult.OK;
			}
			items.add(new Item(this, "/.."));
		}
		else
		{
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
				if(Items().length == 0)
				{
					return ParseURL("", items, "", progress);
				}
				else
				{
					for(String i : Items())
					{
						items.add(new Item(this, i));
					}
				}
				return GetItemsResult.OK;
			}
		}
		int idx = 0;
		String n = "/" + path.getName();
		for(String i : Items())
		{
			if(i.equals(n))
			{
				return ParseURL(ItemsURLs()[idx], items, n, progress);
			}
			++idx;
		}
		return GetItemsResult.FAIL;
	}
	protected GetItemsResult ParseURL(final String _url, List<Item> items, final String _name, FileSelector.Progress progress)
	{
		String s = LoadText(FullURL(_url), TextEncoding(), progress);
		if(s == null)
			return GetItemsResult.UNABLE_CONNECT;
		if(progress.Canceled())
			return GetItemsResult.CANCELED;
		return ParseText(s, items, _name, progress);
	}
	abstract public GetItemsResult ParseText(final String _text, List<Item> items, final String _name, FileSelector.Progress progress);
	protected boolean LoadFile(final String _url, final File _name, FileSelector.Progress progress)
	{
		try
		{
			File path = _name.getParentFile();
			path.mkdirs();
			File file_tmp = new File(_name.getPath() + ".tmp");
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
				return file_tmp.renameTo(_name);
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
	protected String LoadText(final String _url, final String _encoding, FileSelector.Progress progress)
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
	protected ApplyResult OpenFile(final String url, final File name, FileSelector.Progress progress)
	{
		if(!LoadFile(url, name, progress))
			return ApplyResult.UNABLE_CONNECT2;
		if(progress.Canceled())
			return ApplyResult.CANCELED;
		return Emulator.the.Open(name.toString()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
	}
}
