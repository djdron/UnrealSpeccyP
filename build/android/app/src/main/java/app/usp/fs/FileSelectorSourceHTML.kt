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

import java.util.regex.Pattern

abstract class FileSelectorSourceHTML : FileSelectorSourceWEB() {

	abstract fun Patterns(): Array<String>

	abstract fun PatternGet(items: MutableList<Item>, m: java.util.regex.Matcher, name: String)

	override fun ParseText(text: String, items: MutableList<Item>, name: String, progress: FileSelector.Progress): GetItemsResult {

		var ok = false

		for (p in Patterns()) {
			val pt = Pattern.compile(p)
			val m = pt.matcher(text)

			while (m.find()) {
				ok = true
				PatternGet(items, m, name)
			}
		}

		return if (ok) GetItemsResult.OK else GetItemsResult.INVALID_INFO
	}
}
