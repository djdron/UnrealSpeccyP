module = null;
statusText = 'unknown';
browser_path = "/";

document.addEventListener('DOMContentLoaded', function () {
	var listener = document.getElementById('listener');
	listener.addEventListener('load', moduleDidLoad, true);
	listener.addEventListener('message', handleMessage, true);
	document.getElementById('reset').addEventListener('click', onReset, true);
	document.getElementById('joystick').addEventListener('change', onJoystick, true);
	document.getElementById('zoom').addEventListener('change', onZoom, true);
	document.getElementById('filtering').addEventListener('click', onFiltering, true);
	document.getElementById('black and white').addEventListener('click', onBlackAndWhite, true);
	document.getElementById('gigascreen').addEventListener('click', onGigaScreen, true);
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
			module.postMessage('joystick:' + joy);
		var z = localStorage["zoom"];
		if(z)
			module.postMessage('zoom:' + z);
		var f = localStorage["filtering"];
		if(f)
			module.postMessage('filtering:' + f);
		var bw = localStorage["black and white"];
		if(bw)
			module.postMessage('black and white:' + bw);
		var giga = localStorage["gigascreen"];
		if(giga)
			module.postMessage('gigascreen:' + giga);
	}
}

function optionSelect(o)
{
	var v = localStorage[o];
	if(v)
	{
		var el = document.getElementById(o);
		for(var i = 0; i < el.children.length; i++)
		{
			var c = el.children[i];
			if(c.value == v)
			{
				c.selected = "true";
				break;
			}
		}
	}
}

function pageDidLoad()
{
	if(module == null)
		updateStatus('loading...');
	else
		updateStatus();
	optionSelect("joystick");
	optionSelect("zoom");
	var f = localStorage["filtering"];
	if(f)
	{
		var e = document.getElementById("filtering");
		e.checked = (f == "on");
	}
	var bw = localStorage["black and white"];
	if(bw)
	{
		var e = document.getElementById("black and white");
		e.checked = (bw == "on");
	}
	var giga = localStorage["gigascreen"];
	if(giga)
	{
		var e = document.getElementById("gigascreen");
		e.checked = (giga == "on");
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

function updateBrowserAns(url, text, item)
{
	var path = getPath(url);
	var browser = document.getElementById("browser");
	var items_html = "<li id=\"..\"><b>..</b></li>";
//	alert(html);
	items_html += item.parse_all(item, path, text);
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

function parser_std(path, elem)
{
	var item_name = replace_in_str(res[2], "\\s+", " ");
	var item_desc = "";
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
	var item_url = path + res[1];
	return "<li id=\"" + item_url + "\"><b>" + item_name + "</b><br /><sup>" + item_desc + "</sup></li>";
}

function url_std(u)
{
	return "https://vtrd.in" + u + ".htm";
}

function parse_patterns(item, path, html)
{
	var items_html = "";
	for(i = 0; i < item.patterns.length; ++i)
	{
		var re = new RegExp(item.patterns[i], "g");
		while((res = re.exec(html)) != null)
		{
			items_html += item.parser(path, res);
		}
	}
	return items_html;
}

function replace_in_str(str, s1, s2)
{
	while((str1 = str.replace(s1, s2)) != str)
		str = str1;
	return str;
}

var browser_items =
[
	{
		root:		"/games",
		items:		[ "/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z" ],
		urls:		[ "full_ver", "demo_ver", "translat", "remix", "123", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" ],
		patterns:	[ "<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td>" ],
		parser:		parser_std,
		parse_all:	parse_patterns,
		url:		function(u) { return "https://vtrd.in/games.php?t=" + u; }
	},
	{
		root:		"/demos",
		items:		[ "/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z" ],
		urls:		[ "0", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" ],
		patterns:	[ "" ],
		parser:		parser_std,
		parse_all:	function(item, path, json)
					{
						json = replace_in_str(json, "\\'", "'");
						json = replace_in_str(json, "\t", " ");
						if(json.length > 0 && json[json.length - 1] == ",")
							json = json.slice(0, -1);
						json = "[" + json + "]";
						var items_html = "";
						try
						{
							var items = JSON.parse(json);
							for(var i = 0; i < items.length; i++)
							{
								var item = items[i];
								var author = item.author != "???" ? item.author : "";
								var city = item.city != "???" ? item.city : "";
								var year = item.year != "0000" ? item.year : "";
								var desc = author;
								if(desc.length > 0 && city.length > 0)
									desc += " / ";
								desc += city;
								if(desc.length > 0 && year.length > 0)
									desc += " ' ";
								desc += year;
								items_html += "<li id=\"" + item.url + "\"><b>" + item.title + "</b><br /><sup>" + desc + "</sup></li>";
							}
						}
						catch(err) {}
						return items_html;
					},
		url:		function(u) { return "http://bbb.retroscene.org/unreal_demos.php?l=" + u; }
	},
	{
		root:		"/press",
		items:		[ "/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z" ],
		urls:		[ "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2" ],
		patterns:	[ "<td class=\"nowrap\"><b>(.+?)</b></td>\n<td>([\\s\\S]+?)\n</td>\n</tr>" ],
		parser:		function parser_press(path, elem)
					{
						var browser_name = browser_path.substring(getPath(browser_path).length);
						var press123 = browser_name == "123";
						var name = elem[1];
						if(press123)
						{
							if(isLetter(name.substring(0, 1)))
								return "";
						}
						else if(browser_name[0] != name[0])
							return "";
						var re1 = new RegExp("<a class=\"rpad\" href=\"(.+?)\">(.+?)</a>", "g");
						var item_html = "";
						while((res1 = re1.exec(elem[2])) != null)
						{
							var item_name = name + " - " + res1[2];
							var item_url = path + res1[1];
							item_html += "<li id=\"" + item_url + "\"><b>" + item_name + "</b></li>";
						}
						return item_html;
					},
		parse_all:	parse_patterns,
		url:		function(u) { return "https://vtrd.in/press.php?l=" + u; }
	},
	{
		root:		"/replays",
		items:		[ "/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z" ],
		urls:		[ "0", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" ],
		patterns:	[ "<font size=2>(.+?)(?:<br>.+?|</td>)<td align=center><font size=2>(.+?)</td><td align=center>(?:<font size=1><A HREF=\"(.+?)\"|<font size=2 color=red>).+?" ],
		parser:		function parser_press(path, elem)
					{
						if(elem[3] == null)
							return ""
						var item_name = elem[1];
						var item_desc = elem[2];
						var item_url = path + elem[3];
						return "<li id=\"" + item_url + "\"><b>" + item_name + "</b><br /><sup>" + item_desc + "</sup></li>";
					},
		parse_all:	parse_patterns,
		url:		function(u) { return "http://rzxarchive.co.uk/" + u + ".php"; }
	},
];

function updateBrowser()
{
	localStorage["browser_path"] = browser_path;
	if(browser_path == "/")
	{
		var items = [];
		for(var i = 0; i < browser_items.length; ++i)
		{
			items.push(browser_items[i].root);
		}
		updateBrowserItems(items, true);
		return;
	}
	else
	{
		for(var i = 0; i < browser_items.length; ++i)
		{
			if(browser_path == browser_items[i].root)
			{
				updateBrowserItems(browser_items[i].items, false);
				return;
			}
		}
	}

	var path = getPath(browser_path);
	for(var i = 0; i < browser_items.length; ++i)
	{
		if(path == browser_items[i].root + "/")
		{
			var name = browser_path.substring(path.length - 1);
			var u = browser_items[i].items.indexOf(name);
			if(u >= 0)
			{
				var browser = document.getElementById("browser");
				browser.innerHTML = "<li>gathering list...</li>";
				var url = browser_items[i].url(browser_items[i].urls[u]);
				var httprq = new XMLHttpRequest();
				httprq.open("GET", url, true);
				httprq.onreadystatechange = function()
				{
					if(httprq.readyState==4 && httprq.status==200)
						updateBrowserAns(url, httprq.responseText, browser_items[i]);
				}
				httprq.send();
			}
			return;
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

function onZoom()
{
	var zoom = document.getElementById("zoom");
	var z = zoom.options[zoom.selectedIndex].value;
	module.postMessage('zoom:' + z);
	localStorage["zoom"] = z;
}

function onFiltering()
{
	var filtering = document.getElementById("filtering");
	var v = filtering.checked ? "on" : "off";
	module.postMessage('filtering:' + v);
	localStorage["filtering"] = v;
}

function onBlackAndWhite()
{
	var bw = document.getElementById("black and white");
	var v = bw.checked ? "on" : "off";
	module.postMessage('black and white:' + v);
	localStorage["black and white"] = v;
}

function onGigaScreen()
{
	var giga = document.getElementById("gigascreen");
	var v = giga.checked ? "on" : "off";
	module.postMessage('gigascreen:' + v);
	localStorage["gigascreen"] = v;
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
