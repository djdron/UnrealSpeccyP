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
	boolean LongUpdate() { return PathLevel(State().current_path) >= 2; }
    @Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		sources.add(new ParserGames());
		sources.add(new ParserDemos());
		sources.add(new ParserPress());
	}
	abstract class FSSVtrdos extends FSSHtml
	{
		private static final String VTRDOS_FS = "/sdcard/usp/vtrdos";
		public String BaseURL() { return "http://vtrdos.ru"; }
		public String HtmlExt() { return ".htm"; }
		public String HtmlEncoding() { return "windows-1251"; }
		public ApplyResult ApplyItem(Item item)
		{
			try
			{
				String p = item.url;
				File file = new File(VTRDOS_FS + p).getCanonicalFile();
				if(!LoadFile(BaseURL() + p, file))
					return ApplyResult.UNABLE_CONNECT2;
				return Emulator.the.Open(file.getAbsolutePath()) ? ApplyResult.OK : ApplyResult.UNSUPPORTED_FORMAT;
			}
			catch(Exception e)
			{
			}
			return ApplyResult.FAIL;
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
			{	"/Russian", "/Other", "/Demobit'1995", "/Enlight'1996", "/Enlight'1997", "/Doxycon'1998", "/Funtop'1998",
				"/Doxycon'1999", "/Chaos Constructions'1999", "/Paradox'1999", "/CAFe'1999", "/Di:Halt'1999", "/Phat9",
				"/Forever 2e3", "/Millenium'1900", "/ZX-Party 2000", "/Paradox'2k", "/Chaos Constructions'2000", "/Phat0",
				"/Forever 2e3SE", "/ASCii'2001", "/Millenium'1901", "/Paradox'2k+1", "/Chaos Constructions'2001", "/Phat1",
				"/Forever 3", "/Millenium'1902", "/Twilight'2002", "/CAFe'2002", "/ASCii'2002", "/Paradox'2002", "/International Vodka Party'2002",
				"/Forever 4", "/ASCii'2003", "/ParaDIGMus'2003", "/Syndeecate Apocalypse'2003", "/CAFe'2003", "/International Vodka Party'2003", "/Millenium'1903",
				"/Forever 5", "/Chaos Constructions'2004", "/ASCii'2004", "/Kidsoft'2004",
				"/Raww Orgy'2005", "/Forever 6", "/Assembly 2005", "/DiHalt'2005", "/Chaos Constructions'2005", "/ZX-Spectrum Party'2005",
				"/Raww Orgy'2006", "/Forever 7", "/International Vodka Party'2006", "/DiHalt'2006", "/Chaos Constructions'2006", "/Sundown'2006", "/ArtFiled'2006",
				"/Raww Orgy'2007", "/Forever 8", "/Di:Halt'2007", "/CCA'2007", "/Sundown'2007", "/ASCii'2007", "/ArtFiled'2007",
				"/Raww Orgy'2008", "/Forever 9", "/Di:Halt'2008", "/International Vodka Party'2008", "/RetroEuskal'2008", "/Arok 10", "/Chaos Constructions'2008", "/ASCii'2008", "/ArtFiled'2008",
				"/Raww Orgy'2009", "/Forever10", "/ArtField'2009", "/HT'2009 SE", "/DiHalt'2009", "/CC'2009", "/International Vodka Party'2009",
				"/Raww Orgy'2010", "/FOReVER:2010", "/HT'2010 SE", "/ArtField'2010", "/DiHalt'2010", "/Chaos Constructions'2010", "/tUM'2010",
				"/FOReVER C", "/ArtField'2011", "/DiHalt'2011", "/CC'2011", "/IVP'2011",
			};
		private final String[] ITEMS2URLS = new String[]
			{	"/demos/russian", "/demos/other", "/demos/demob95", "/demos/enl96", "/demos/enl97", "/demos/doxy98", "/demos/ft98",
				"/demos/doxy99", "/demos/cc999", "/demos/pdox99", "/demos/cafe99", "/demos/dihalt99", "/demos/phat9",
				"/demos/forev2e3", "/demos/mln00", "/demos/zxp2000", "/demos/pdx00", "/demos/cc00", "/demos/phat0",
				"/demos/for2e3se", "/demos/ascii01", "/demos/mln1901", "/demos/pdx01", "/demos/cc01", "/demos/phat1",
				"/demos/forever3", "/demos/mln1902", "/demos/twilight", "/demos/cafe02", "/demos/ascii02", "/demos/pdx02", "/demos/ivp02",
				"/demos/forever4", "/demos/ascii03", "/demos/paradig3", "/demos/synd03", "/demos/cafe03", "/demos/ivp03", "/demos/mln1903",
				"/demos/forever5", "/demos/cc04", "/demos/ascii04", "/demos/kidsoft4",
				"/demos/r_orgy05", "/demos/forever6", "/demos/asm2005", "/demos/dihalt05", "/demos/cc05", "/demos/kidsoft5",
				"/demos/r_orgy06", "/demos/forever7", "/demos/ivp06", "/demos/dihalt06", "/demos/cc06", "/demos/sd2006", "/demos/kidsoft6",
				"/demos/r_orgy07", "/demos/forever8", "/demos/dihalt07", "/demos/cca07", "/demos/sd2007", "/demos/ascii07", "/demos/kidsoft7",
				"/demos/r_orgy08", "/demos/forever9", "/demos/dihalt08", "/demos/ivp08", "/demos/reuskal8", "/demos/arok10", "/demos/cc08", "/demos/ascii08", "/demos/kidsoft8",
				"/demos/r_orgy09", "/demos/foreverx", "/demos/kidsoft9", "/demos/ht09se", "/demos/dihalt09", "/demos/cc09", "/demos/ivp09",
				"/demos/r_orgy10", "/demos/foreve10", "/demos/ht10se", "/demos/kidsof10", "/demos/dihalt10", "/demos/cc10", "/demos/tum10",
				"/demos/foreverc", "/demos/kidsof11", "/demos/dihalt11", "/demos/cc11", "/demos/ivp11",
			};
		private final String[] PATTERNS = new String[]
			{	"<div align=\"center\">.*(?:<b>)?<a href=\"(.+?)\">([\\s\\S]+?)(?:</a>)?(?:</b>)?</div>",
				"<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td>\\s+<td>(.+)</td>",
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
			item.name = item.name.replaceAll("\\s+", " ");
			if(m.groupCount() > 2)
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
