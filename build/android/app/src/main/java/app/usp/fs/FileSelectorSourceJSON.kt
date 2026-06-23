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

import org.json.JSONArray
import org.json.JSONObject
import org.json.JSONTokener

abstract class FileSelectorSourceJSON : FileSelectorSourceWEB() {

	abstract fun JsonGet(items: MutableList<Item>, ji: JSONObject, name: String)

	open fun TextToJSONArray(_text: String): JSONArray? {
		return try {
			JSONTokener(_text).nextValue() as? JSONArray
		} catch (e: Exception) {
			null
		}
	}

	override fun ParseText(text: String, items: MutableList<Item>, name: String, progress: FileSelector.Progress): GetItemsResult {

		val jitems = TextToJSONArray(text) ?: return GetItemsResult.INVALID_INFO

		for (i in 0 until jitems.length()) {
			try {
				JsonGet(items, jitems.getJSONObject(i), name)
			} catch (e: Exception) {
			}
		}
		return GetItemsResult.OK
	}
}
