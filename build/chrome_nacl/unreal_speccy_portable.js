module = null;
statusText = 'unknown';
browser_path = "/";

document.addEventListener('DOMContentLoaded', function () {
	var listener = document.getElementById('listener')
	listener.addEventListener('load', moduleDidLoad, true);
	listener.addEventListener('message', handleMessage, true);
	pageDidLoad();
});

function moduleDidLoad()
{
	module = document.getElementById('unreal_speccy_portable');
	updateStatus('module_ok');
}

// Handle a message coming from the NaCl module.
function handleMessage(m)
{
	updateStatus(m.data);
	if(m.data == 'resources_ok')
		module.postMessage('run');
	else if(m.data == 'ready')
	{
		var joy = localStorage["joystick"];
		if(joy)
		{
			module.postMessage('joystick:' + joy);
		}
	}
}

function pageDidLoad()
{
	if(module == null)
		updateStatus('loading...');
	else
		updateStatus();
	var joy = localStorage["joystick"];
	if(joy)
	{
		var joystick = document.getElementById("joystick");
		for(var i = 0; i < joystick.children.length; i++)
		{
			var j = joystick.children[i];
			if(j.value == joy)
			{
				j.selected = "true";
				break;
			}
		}
	}
	var bp = localStorage["browser_path"];
	if(bp)
		browser_path = bp;
	updateBrowser();
}

function getPath(name)
{
	var pos = name.lastIndexOf('/');
	if(pos == -1)
		return "";
	return name.substring(0, pos + 1);
}

function installBrowserCallbacks()
{
	var browser = document.getElementById("browser");
	var lis = browser.getElementsByTagName("li");
	for(i = 0; i < lis.length; ++i)
	{
		lis[i].onmouseover = function() { this.style.backgroundColor = "#EEE"; }
		lis[i].onmouseout = function() { this.style.backgroundColor = ""; }
		lis[i].onclick = function()
		{
			this.style.backgroundColor = "#CCC";
			if(this.id[0] == "/")
			{
				if(browser_path == "/")
					browser_path = this.id;
				else
					browser_path += this.id;
				updateBrowser();
			}
			else if(this.id == "..")
			{
				browser_path = getPath(browser_path);
				if(browser_path.length > 1 && browser_path[browser_path.length - 1] == '/')
					browser_path = browser_path.substring(0, browser_path.length - 1);
				updateBrowser();
			}
			else
			{
				updateStatus('opening...');
				module.postMessage('open:' + this.id);
			}
		}
	}
}

function isLetter(str)
{
	return str.length === 1 && str.match(/[a-z]/i);
}

function updateBrowserAns(url, html, patterns)
{
	var path = getPath(url);
	var browser = document.getElementById("browser");
	var items_html = "<li id=\"..\"><b>..</b></li>";
//	alert(html);
	var browser_name = browser_path.substring(getPath(browser_path).length);
	var press = (patterns == patterns_press);
	var press123 = press && (browser_name == "123");
	for(i = 0; i < patterns.length; ++i)
	{
		var re = new RegExp(patterns[i], "g");
		while((res = re.exec(html)) != null)
		{
			var item_name = null;
			var item_url = null;
			var item_desc = "";
			if(!press)
			{
				item_name = res[2];
				while((name1 = item_name.replace("\\s+", " ") != item_name))
					item_name = name1;
				if(res.length > 3)
				{
					item_desc = res[3];
					if(res.length > 4)
					{
						item_desc += " / " + res[4];
						if(res.length > 5)
						  item_desc += " [" + res[5] + "]";
					}
				}
				item_url = path + res[1];
				items_html += "<li id=\"" + item_url + "\"><b>" + item_name + "</b><br /><sup>" + item_desc + "</sup></li>";
			}
			else // press
			{
				var name = res[1];
				if(press123)
				{
					if(isLetter(name.substring(0, 1)))
						continue;
				}
				else if(browser_name[0] != name[0])
					continue;
				var re1 = new RegExp("<a href=\"(.+)\">(.+)</a>", "g");
				while((res1 = re1.exec(res[2])) != null)
				{
					item_name = name + " - " + res1[2];
					item_url = path + res1[1];
					items_html += "<li id=\"" + item_url + "\"><b>" + item_name + "</b></li>";
				}
			}
		}
	}
	browser.innerHTML = items_html;
	installBrowserCallbacks();
}

function updateBrowserItems(items, root)
{
	var items_html = "";
	if(!root)
	{
		items_html += "<li id=\"..\"><b>..</b></li>";
	}
	for(i = 0; i < items.length; ++i)
	{
		items_html += "<li id=\"" + items[i] + "\"><b>" + items[i] + "</b></li>";
	}
	browser.innerHTML = items_html;
	installBrowserCallbacks();
}

var items_browser_root = [ "/games", "/demos", "/press" ];
var items_browser_games = [ "/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B",
				"/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V",
				"/W", "/X", "/Y", "/Z" ];
var urls_games = [ "/g/game_full_ver", "/g/game_demo_ver", "/g/game_translat",
				"/g/game_remix", "/g/game_123", "/g/game_a", "/g/game_b",
				"/g/game_c", "/g/game_d", "/g/game_e", "/g/game_f",
				"/g/game_g", "/g/game_h", "/g/game_i", "/g/game_j",
				"/g/game_k", "/g/game_l", "/g/game_m", "/g/game_n",
				"/g/game_o", "/g/game_p", "/g/game_q", "/g/game_r",
				"/g/game_s", "/g/game_t", "/g/game_u", "/g/game_v",
				"/g/game_w", "/g/game_x", "/g/game_y", "/g/game_z" ];
var patterns_games = [ "<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td>" ];

var items_browser_demos = [ "/Russian", "/Other", "/Demobit'1995", "/Enlight'1996", "/Enlight'1997", "/Doxycon'1998", "/Funtop'1998",
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
				"/FOReVER C", "/ArtField'2011", "/DiHalt'2011", "/CC'2011", "/IVP'2011" ];
var urls_demos = [ "/demos/russian", "/demos/other", "/demos/demob95", "/demos/enl96", "/demos/enl97", "/demos/doxy98", "/demos/ft98",
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
				"/demos/foreverc", "/demos/kidsof11", "/demos/dihalt11", "/demos/cc11", "/demos/ivp11" ];
var patterns_demos = [ "<div align=\"center\">.*(?:<b>)?<a href=\"(.+?)\">([\\s\\S]+?)(?:</a>)?(?:</b>)?</div>",
				"<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td>\\s+<td>(.+)</td>",
				"<a href=\"(.+)\">(.+)</a></td>\\s+<td width=\"45%\">(.+)</td>" ];

var items_browser_press = [ "/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
				"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z" ];
var urls_press = [ "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an", "/press/press_an",
				"/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz", "/press/press_oz" ];
var patterns_press = [ "<td>&nbsp;&nbsp;<b>(.+)</b></td>([\\s\\S]+?)</td></tr>" ];

function updateBrowser()
{
	localStorage["browser_path"] = browser_path;
	var items = null;
	var root = false;
	if(browser_path == "/")
	{
		items = items_browser_root;
		root = true;
	}
	else if(browser_path == "/games")
		items = items_browser_games;
	else if(browser_path == "/demos")
		items = items_browser_demos;
	else if(browser_path == "/press")
		items = items_browser_press;

	if(items != null)
	{
		updateBrowserItems(items, root);
	}
	else
	{
		var urls = null;
		var patterns = null;
		var path = getPath(browser_path);
		if(path == "/games/")
		{
			items = items_browser_games;
			urls = urls_games;
			patterns = patterns_games;
		}
		else if(path == "/demos/")
		{
			items = items_browser_demos;
			urls = urls_demos;
			patterns = patterns_demos;
		}
		else if(path == "/press/")
		{
			items = items_browser_press;
			urls = urls_press;
			patterns = patterns_press;
		}
		if(items != null)
		{
			var name = browser_path.substring(path.length - 1);
			var i = items.indexOf(name);
			if(i >= 0)
			{
				var browser = document.getElementById("browser");
				browser.innerHTML = "<li>gathering list...</li>";
				var url = "http://vtrdos.ru" + urls[i] + ".htm";
				var httprq = new XMLHttpRequest();
				httprq.open("GET", url, true);
				httprq.onreadystatechange = function()
				{
					if(httprq.readyState==4 && httprq.status==200)
						updateBrowserAns(url, httprq.responseText, patterns);
				}
				httprq.send();
			}
		}
	}
}

function onReset()
{
	module.postMessage('reset');
}

function onJoystick()
{
	var joystick = document.getElementById("joystick");
	var joy = joystick.options[joystick.selectedIndex].value;
	module.postMessage('joystick:' + joy);
	localStorage["joystick"] = joy;
}

function updateStatus(opt_message)
{
	if(opt_message)
		statusText = opt_message;
	var status = document.getElementById('status');
	if(status)
	{
		status.innerHTML = statusText;
	}
}
