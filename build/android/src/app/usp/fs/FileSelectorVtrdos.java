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
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Bundle;
import app.usp.Emulator;

public class FileSelectorVtrdos extends FileSelector
{
	private static State state = new State();
	@Override
	State State() { return state; }
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
		super.onCreate(savedInstanceState);
		sources.add(new ParserGames());
		sources.add(new ParserDemos());
		sources.add(new ParserPress());
    }
	abstract class FSSVtrdos extends FSSWeb
	{
		private static final String VTRDOS_FS = "/sdcard/usp/vtrdos";
		public String BaseURL() { return "http://vtrdos.ru"; }
		public boolean ApplyItem(Item item)
		{
			try
			{
				String p = item.url;
				File file = new File(VTRDOS_FS + p).getCanonicalFile();
				File path = file.getParentFile();
				path.mkdirs();
				FileOutputStream os = new FileOutputStream(file);
				URL url = new URL(BaseURL() + p);
				InputStream is = url.openStream();
				byte buffer[] = new byte[16384];
				int r = 0;
				while((r = is.read(buffer)) != -1)
				{
					os.write(buffer, 0, r);
				}
				is.close();
				os.close();
				Emulator.the.Open(file.getAbsolutePath());
				return true;
			}
			catch(Exception e)
			{
			}
			return false;
		}
	}
	class ParserGames extends FSSVtrdos
	{
		private final String[] ITEMS2 = new String[]
			{	"/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B",
				"/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V",
				"/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
		    {	"/g/game_full_ver", "/g/game_demo_ver", "/g/game_translat",
				"/g/game_remix", "/g/game_123", "/g/game_a", "/g/game_b",
				"/g/game_c", "/g/game_d", "/g/game_e", "/g/game_f",
				"/g/game_g", "/g/game_h", "/g/game_i", "/g/game_j",
				"/g/game_k", "/g/game_l", "/g/game_m", "/g/game_n",
				"/g/game_o", "/g/game_p", "/g/game_q", "/g/game_r",
				"/g/game_s", "/g/game_t", "/g/game_u", "/g/game_v",
				"/g/game_w", "/g/game_x", "/g/game_y", "/g/game_z"
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
			File f = new File(url);
			item.url = f.getParent() + "/" + m.group(1);
			items.add(item);
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
	class ParserDemos extends FSSVtrdos
	{
		private final String[] ITEMS2 = new String[]
			{	"/Russian", "/Other", "/Enlight'1996", "/Enlight'1997", "/Funtop'1998", "/Chaos Constructions'1999",
				"/Paradox'1999", "/CAFe'1999", "/Chaos Constructions'2000", "/ASCii'2001", "/Chaos Constructions'2001", "/ASCii'2002", "/CAFe'2003"
			};
		private final String[] ITEMS2URLS = new String[]
			{	"/demos/russian", "/demos/other", "/demos/enl96", "/demos/enl97", "/demos/ft98", "/demos/cc999",
				"/demos/pdox99", "/demos/cafe99", "/demos/cc00", "/demos/ascii01", "/demos/cc01", "/demos/ascii02", "/demos/cafe03"
			};
		private final String[] PATTERNS = new String[]
			{	"<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td>\\s+<td>(.+)</td>",
				"<a href=\"(.+)\">(.+)</a></td>\\s+<td width=\"45%\">(.+)</td>",
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
			item.desc = m.group(3);
			File f = new File(url);
			item.url = f.getParent() + "/" + m.group(1);
			items.add(item);
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
	class ParserPress extends FSSVtrdos
	{
		private final String[] ITEMS2 = new String[]
			{	"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
			};
		private final String[] ITEMS2URLS = new String[]
			{	"/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an",
				"/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz"
			};
		private final String[] PATTERNS = new String[]
			{	"<td>&nbsp;&nbsp;<b>(.+)</b></td>(?s)(.+?)</td></tr>" };
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

			Pattern pt = Pattern.compile("<a href=\"(.+)\">(.+)</a>");
			Matcher m2 = pt.matcher(m.group(2));
			while(m2.find())
			{
				Item item = new Item();
				item.name = name + " - " + m2.group(2);
				File f = new File(url);
				item.url = f.getParent() + "/" + m2.group(1);
				items.add(item);
			}
		}
		@Override
		public final String[] Items2() { return ITEMS2; }
		@Override
		public final String[] Items2URLs() { return ITEMS2URLS; }
	}
}
