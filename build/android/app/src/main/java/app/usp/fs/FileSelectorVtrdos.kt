/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

package app.usp.fs

import java.io.File
import java.util.regex.Pattern
import android.os.Bundle
import org.json.JSONObject
import app.usp.R

class FileSelectorVtrdos : FileSelector() {

	companion object {
		private val state = State()
		const val PARSER_UPDATES_ROOT: String = "/updates"
	}

	override fun getState(): State = state

	override fun LongUpdate(path: File): Boolean = PathLevel(path) >= 2 || path.path == PARSER_UPDATES_ROOT

	override fun LongUpdateTitle(): Int = R.string.accessing_web

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		sources.add(ParserGames())
		sources.add(ParserDemos())
		sources.add(ParserPress())
		sources.add(ParserUpdates())
	}

	val baseUrl: String = "https://vtrd.in"

	inner abstract class FSSVtrdosHTML() : FileSelectorSourceHTML() {
		protected val VTRDOS_FS: String = context?.filesDir?.toString() + "/vtrdos"

		override fun TextEncoding(): String = "utf-8"

		override fun ApplyItem(item: Item, progress: FileSelector.Progress): ApplyResult {
			val file = File(VTRDOS_FS + item.url)
			return OpenFile(baseUrl + item.url, file, progress)
		}
	}
	inner class ParserGames : FSSVtrdosHTML() {

		private val ITEMS = arrayOf(
			"/russian", "/demo", "/translate", "/remix", "/123", "/A", "/B",
			"/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
			"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V",
			"/W", "/X", "/Y", "/Z"
		)

		private val ITEMSURLS = arrayOf(
			"full_ver", "demo_ver", "translat", "remix", "123",
			"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
			"n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
		)

		private val PATTERNS = arrayOf(
			"<a href=\"(.+)\">&nbsp;&nbsp;(.+)</a></td><td>(.+)</td><td>(.+)</td><td>(.+)</td><td>.+</a></td>"
		)

		override fun FullURL(url: String): String = "$baseUrl/games.php?t=$url"

		override fun Root(): String = "/games"

		override fun Patterns(): Array<String> = PATTERNS

		override fun PatternGet(items: MutableList<Item>, m: java.util.regex.Matcher, name: String) {
			val name = m.group(2) ?: return
			val href = m.group(1) ?: return
			val item = Item(this, name)
			item.desc = "${m.group(3)} / ${m.group(4)} [${m.group(5)}]"
			item.url = href
			items.add(item)
		}

		override fun Items(): Array<String> = ITEMS

		override fun ItemsURLs(): Array<String> = ITEMSURLS
	}
	inner class ParserDemos : FSSVtrdosHTML() {

		private val ITEMS = arrayOf(
			"/Russian", "/Other", "/1995", "/1996", "/1997", "/1998", "/1999", "/2000", "/2001",
			"/2002", "/2003", "/2004", "/2005", "/2006", "/2007", "/2008", "/2009", "/2010",
			"/2011", "/2012", "/2013", "/2014", "/2015", "/2016", "/2017", "/2018", "/2019", "/2020",
			"/2021", "/2022", "/2023", "/2024"
		)

		private val ITEMSURLS = arrayOf(
			"/russian", "/other", "1995", "1996", "1997", "1998", "1999", "2000", "2001",
			"2002", "2003", "2004", "2005", "2006", "2007", "2008", "2009", "2010",
			"2011", "2012", "2013", "2014", "2015", "2016", "2017", "2018", "2019", "2020",
			"2021", "2022", "2023", "2024"
		)

		private val PATTERNS = arrayOf(
			"<a href=\"(.+?)\" target=\"demozdown\">(.+?)</a>",
			"<a href=\"(.+?)\">(.+?)</a></td><td>(.+?)</td>"
		)

		private val DEMO_SIGN = "demo.php?party="

		override fun FullURL(url: String): String {
			return if (url.isNotEmpty() && url[0] == '/') {
				"$baseUrl$url.php"
			} else {
				"$baseUrl/skin/party.php?year=$url"
			}
		}

		override fun Root(): String = "/demos"

		override fun Patterns(): Array<String> = PATTERNS

		override fun PatternGet(items: MutableList<Item>, m: java.util.regex.Matcher, name: String) {
			val name = m.group(2) ?: return
			val href = m.group(1) ?: return
			val item = Item(this, name)
			item.url = "/$href"
			if (m.groupCount() > 2) {
				item.desc = m.group(3)
			}
			items.add(item)
		}

		override fun Items(): Array<String> = ITEMS

		override fun ItemsURLs(): Array<String> = ITEMSURLS

		override fun ApplyItem(item: Item, progress: FileSelector.Progress): ApplyResult {
			val url = item.url ?: return ApplyResult.FAIL
			val idx = url.indexOf(DEMO_SIGN)
			if (idx < 0) {
				return super.ApplyItem(item, progress)
			}

			try {
				val subUrl = url.substring(idx + DEMO_SIGN.length)
				val s = LoadText("$baseUrl/$DEMO_SIGN$subUrl", TextEncoding(), progress)
					?: return ApplyResult.UNABLE_CONNECT1

				if (progress.Canceled()) return ApplyResult.CANCELED

				val pt = Pattern.compile("<a target=\"_blank\" href=\"(.+?)\">(.+?)</a>")
				val m = pt.matcher(s)

				if (m.find()) {
					val name = m.group(2) ?: return ApplyResult.FAIL
					val href = m.group(1) ?: return ApplyResult.FAIL
					val item2 = Item(this, name)
					if (href.startsWith("../")) {
						item2.url = "/" + href.substring(3)
					} else {
						item2.url = "/$href"
					}
					return super.ApplyItem(item2, progress)
				}
			} catch (e: Exception) {
			}
			return ApplyResult.FAIL
		}
	}
	inner class ParserPress : FSSVtrdosHTML() {

		private val ITEMS = arrayOf(
			"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
			"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
		)

		private val ITEMSURLS = arrayOf(
			"1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1", "1",
			"2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2", "2"
		)

		private val PATTERNS = arrayOf(
			"<td class=\"nowrap\"><b>(.+?)</b></td>\n<td>(?s)(.+?)</td>\n</tr>"
		)

		override fun FullURL(url: String): String = "$baseUrl/press.php?l=$url"

		override fun Root(): String = "/press"

		override fun Patterns(): Array<String> = PATTERNS

		override fun PatternGet(items: MutableList<Item>, m: java.util.regex.Matcher, name: String) {
			val namep = m.group(1) ?: return
			if (name.length < 2 || namep.isEmpty()) return

			val ch1 = name[1]
			val ch0 = namep[0]

			if (ch1.isDigit()) {
				if (ch0.isLetter()) return
			} else if (ch0 != ch1) {
				return
			}

			val pt = Pattern.compile("<span style=\"white-space: nowrap;\"><a href=\"(.+?)\">(.+?)</a>")
			val m2 = pt.matcher(m.group(2) ?: "")

			while (m2.find()) {
				val item = Item(this, namep + " - " + m2.group(2))
				item.url = "/" + m2.group(1)
				items.add(item)
			}
		}

		override fun Items(): Array<String> = ITEMS

		override fun ItemsURLs(): Array<String> = ITEMSURLS
	}
	inner class ParserUpdates : FileSelectorSourceJSON() {

		protected val vtrdosFs: String = context?.filesDir?.toString() + "/vtrdos"

		override fun TextEncoding(): String = "utf-8"

		override fun FullURL(url: String): String = "$baseUrl/updates.php?json"

		override fun Root(): String = PARSER_UPDATES_ROOT

		override fun ApplyItem(item: Item, progress: FileSelector.Progress): ApplyResult {
			val p = item.url ?: return ApplyResult.FAIL
			if (!p.startsWith(baseUrl)) return ApplyResult.FAIL

			val file = File(vtrdosFs + p.substring(baseUrl.length))
			return OpenFile(p, file, progress)
		}

		override fun JsonGet(items: MutableList<Item>, ji: JSONObject, name: String) {
			val t = ji.optString("title", "")
			if (t.isEmpty()) return

			val u = ji.optString("url", "")
			if (u.isEmpty()) return

			val size = ji.optLong("filesize", 0)
			if (size > 5 * 1024 * 1024) return

			val item = Item(this, t)
			item.url = u
			item.desc = ji.optString("info", "")
			items.add(item)
		}

		override fun Items(): Array<String> = emptyArray()

		override fun ItemsURLs(): Array<String> = emptyArray()
	}
}
