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
import org.json.JSONObject

import android.os.Bundle
import app.usp.R

class FileSelectorBBB : FileSelector() {

	companion object { private val state = State() }
	override fun getState(): State = state

	override fun LongUpdate(path: File): Boolean = PathLevel(path) >= 1

	override fun LongUpdateTitle(): Int = R.string.accessing_web

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		sources.add(FSSBBB())
	}

	inner class FSSBBB : FileSelectorSourceJSON() {

		private val bbbFs: String = context?.filesDir?.toString() + "/bbb"
		var baseUrl: String = "https://bbb.retroscene.org"

		override fun FullURL(url: String): String = "$baseUrl/unreal_demos.php?l=$url"

		override fun TextEncoding(): String = "iso-8859-1"

		override fun Root(): String? = null

		override fun ApplyItem(item: Item, progress: Progress): ApplyResult {
			val p = item.url ?: return ApplyResult.FAIL
			if (!p.startsWith(baseUrl)) {
				return ApplyResult.FAIL
			}
			val file = File(bbbFs + p.substring(baseUrl.length))
			return OpenFile(p, file, progress)
		}

		override fun ParseText(text: String, items: MutableList<Item>, name: String, progress: Progress): GetItemsResult {
			return super.ParseText("[$text]", items, name, progress)
		}

		override fun JsonGet(items: MutableList<Item>, ji: JSONObject, name: String) {
			val t = ji.optString("title", "")
			if (t.isEmpty()) return

			val u = ji.optString("url", "")
			if (u.isEmpty()) return

			val a = ji.optString("author", "")
			val y = ji.optString("year", "")
			val c = ji.optString("city", "")

			val item = Item(this, t)
			item.url = u
			item.desc = a

			if (y.isNotEmpty()) {
				item.desc += "'$y"
			}
			if (c.isNotEmpty()) {
				item.desc += " / $c"
			}
			items.add(item)
		}

		private val ITEMS = arrayOf(
				"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
				"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
				"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
		)

		private val ITEMSURLS = arrayOf(
				"@", "a", "b", "c", "d", "e", "f", "g", "h",
				"i", "j", "k", "l", "m", "n", "o", "p", "q",
				"r", "s", "t", "u", "v", "w", "x", "y", "z"
		)

		override fun Items(): Array<String> = ITEMS

		override fun ItemsURLs(): Array<String> = ITEMSURLS
	}
}
