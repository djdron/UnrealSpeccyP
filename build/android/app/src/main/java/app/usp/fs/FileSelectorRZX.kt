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
import java.net.URLConnection
import android.os.Bundle
import org.json.JSONArray
import org.json.JSONObject
import org.json.JSONTokener
import app.usp.R

class FileSelectorRZX : FileSelector() {

	companion object {
		private val state = State()
	}

	override fun getState(): State = state

	override fun LongUpdate(path: File): Boolean = PathLevel(path) >= 1

	override fun LongUpdateTitle(): Int = R.string.accessing_web

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		sources.add(FSSRZX())
	}

	inner class FSSRZX : FileSelectorSourceJSON() {

		private val rzxFs: String = context?.filesDir?.toString() + "/rzx"
		protected val baseUrl: String = "https://api.vtrd.in/v1/rzx"

		override fun FullURL(url: String): String = "$baseUrl$url"

		override fun TextEncoding(): String = "iso-8859-1"

		override fun SetupConnection(connection: URLConnection) {
			connection.setRequestProperty(
				"Authorization",
				"Bearer SD0XHmQZzbZpK1PyPbIgfL2P4iaCeQ02gTJogfPH43A88FN6B02dFJbJo0Ho"
			)
		}

		override fun ApplyItem(item: Item, progress: Progress): ApplyResult {
			val url = item.url ?: return ApplyResult.FAIL
			var pos = url.lastIndexOf("/rzx/")
			if (pos == -1)
			{
				pos = url.lastIndexOf("/zxdb/")
				if(pos == -1)
					return ApplyResult.FAIL
				else
					pos += 5
			}
			else
				pos += 4
			val file = File(rzxFs + url.substring(pos))
			return OpenFile(url, file, progress)
		}

		private val ITEMS = arrayOf(
			"/123", "/A", "/B", "/C", "/D", "/E", "/F", "/G", "/H",
			"/I", "/J", "/K", "/L", "/M", "/N", "/O", "/P", "/Q",
			"/R", "/S", "/T", "/U", "/V", "/W", "/X", "/Y", "/Z"
		)

		private val ITEMSURLS = arrayOf(
			"/0", "/a", "/b", "/c", "/d", "/e", "/f", "/g", "/h",
			"/i", "/j", "/k", "/l", "/m", "/n", "/o", "/p", "/q",
			"/r", "/s", "/t", "/u", "/v", "/w", "/x", "/y", "/z"
		)

		override fun Root(): String? = null

		override fun TextToJSONArray(_text: String): JSONArray? {
			return try {
				val json = JSONTokener(_text).nextValue() as JSONObject
				json.optJSONArray("data")
			} catch (e: Exception) {
				null
			}
		}

		override fun JsonGet(items: MutableList<Item>, ji: JSONObject, name: String) {
			val name = ji.optString("name", "")
			if (name.isEmpty()) return

			val url = ji.optString("path", "")
			if (url.isEmpty()) return

			val info = ji.optString("info", "")
			val author = ji.optString("submitter", "")

			val item = Item(this, name)
			item.url = url
			item.desc = author

			if (info.isNotEmpty()) {
				item.desc += " / $info"
			}
			items.add(item)
		}

		override fun Items(): Array<String> = ITEMS

		override fun ItemsURLs(): Array<String> = ITEMSURLS
	}
}
