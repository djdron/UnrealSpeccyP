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

class FileSelectorWOS : FileSelector() {
	companion object { private val state = State() }
	override fun getState(): State = state
	override fun LongUpdate(path: File): Boolean = PathLevel(path) >= 2
	override fun LongUpdateTitle(): Int = R.string.accessing_web

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		sources.addAll(listOf(
			ParserGames(),
			ParserAdventures(),
			//ParserSimulators(),
			ParserEducational(),
			ParserUtilities(),
			ParserDemos(),
			//ParserMisc(),
			//ParserXRated(),
		))
	}

	abstract inner class FSSWOS : FileSelectorSourceJSON() {
		private val WOS_FS: String = (context?.filesDir?.toString() ?: "") + "/wos"
		private val baseUrl = "https://worldofspectrum.org"
		abstract fun URLSection(): String
		override fun FullURL(url: String): String =
			"$baseUrl/software/ajax_software_items/${URLSection()}?columns%5B0%5D%5Bdata%5D=az&columns%5B0%5D%5Bsearch%5D%5Bvalue%5D=$url&columns%5B1%5D%5Bdata%5D=title&columns%5B8%5D%5Bdata%5D=availability&columns%5B8%5D%5Bsearch%5D%5Bvalue%5D=Available&order%5B0%5D%5Bcolumn%5D=1&order%5B0%5D%5Bdir%5D=asc&length=-1"

		override fun TextEncoding(): String = "iso-8859-1"

		private val ITEMS = arrayOf(
			"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H", "/I", "/J", "/K", "/L",
			"/M", "/N", "/O", "/P", "/Q", "/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
		)

		private val ITEMSURLS = arrayOf(
			"%23", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
			"M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
		)

		override fun ParseText(text: String, items: MutableList<Item>, name: String, progress: FileSelector.Progress): GetItemsResult {
			val start = text.indexOf("\"data\":")
			val end = text.lastIndexOf(",\"draw\":")
			if (start < 0 || end < 0) return GetItemsResult.INVALID_INFO
			return super.ParseText(text.substring(start + 7, end), items, name, progress)
		}

		override fun JsonGet(items: MutableList<Item>, ji: JSONObject, name: String) {
			val to = ji.opt("title") ?: return
			val slug = ji.optString("slug", "")
			if (slug.isEmpty()) return

			val item = Item(this, to.toString()).apply { this.url = slug }

			var pub = ji.optString("title_publisher", "")
			if (pub.startsWith(item.name)) pub = pub.substring(item.name.length).trim()
			if (pub.startsWith("(") && pub.endsWith(")")) item.desc = pub.substring(1, pub.length - 1)

			ji.optString("release_year", "").let { y -> if (y.isNotEmpty()) item.desc = (item.desc ?: "") + "'$y" }
			ji.optString("entry_type", "").let { eto ->
				if(eto.isNotEmpty()) {
					if(item.desc.isNullOrEmpty()) item.desc = eto
					else item.desc = "${item.desc} - $eto"
				}
			}
			items.add(item)
		}
		override fun Items(): Array<String> = ITEMS
		override fun ItemsURLs(): Array<String> = ITEMSURLS
		val URL_START = "/pub/sinclair"
		override fun ApplyItem(item: Item, progress: FileSelector.Progress): ApplyResult {
			val itemUrl = item.url ?: return ApplyResult.FAIL

			val s = LoadText("$baseUrl/archive/software/${URLSection()}/$itemUrl", TextEncoding(), progress)
				?: return ApplyResult.UNABLE_CONNECT1

			if (progress.Canceled()) return ApplyResult.CANCELED

			var fileUrl: String? = null
			val pt = Pattern.compile("<th>Play</th>(?s).+?<th>File</th>.+?<a href=\"(.+?)\"")
			val m = pt.matcher(s)

			if (m.find()) {
				fileUrl = m.group(1)
			}

			if (fileUrl == null) return ApplyResult.NOT_AVAILABLE

			val idx = fileUrl.indexOf(URL_START)
			if (idx == -1) return ApplyResult.FAIL

			val p = fileUrl.substring(idx + URL_START.length)
			val f = File(WOS_FS + p)

			return OpenFile(fileUrl, f, progress)
		}
	}
	inner class ParserGames : FSSWOS() {
		override fun URLSection(): String = "games"
		override fun Root(): String = "/games"
	}

	inner class ParserAdventures : FSSWOS() {
		override fun URLSection(): String = "text-adventures"
		override fun Root(): String = "/adventures"
	}

	inner class ParserSimulators : FSSWOS() {
		override fun URLSection(): String = "simulators"
		override fun Root(): String = "/simulators"
	}

	inner class ParserEducational : FSSWOS() {
		override fun URLSection(): String = "educational"
		override fun Root(): String = "/educational"
	}

	inner class ParserUtilities : FSSWOS() {
		override fun URLSection(): String = "utilities"
		override fun Root(): String = "/utilities"
	}

	inner class ParserDemos : FSSWOS() {
		override fun URLSection(): String = "demos"
		override fun Root(): String = "/demos"
	}

	inner class ParserMisc : FSSWOS() {
		override fun URLSection(): String = "miscellaneous"
		override fun Root(): String = "/misc"
	}

	inner class ParserXRated : FSSWOS() {
		override fun URLSection(): String = "xrated"
		override fun Root(): String = "/xrated"
	}
}
