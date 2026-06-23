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

abstract class FileSelectorSource {

	class Item(var source: FileSelectorSource, var name: String) {
		var desc: String? = null
		var url: String? = null
	}

	enum class GetItemsResult { OK, FAIL, UNABLE_CONNECT, INVALID_INFO, CANCELED }
	abstract fun GetItems(path: File, items: MutableList<Item>, progress: FileSelector.Progress): GetItemsResult
	enum class ApplyResult { OK, FAIL, UNABLE_CONNECT1, UNABLE_CONNECT2, INVALID_INFO, NOT_AVAILABLE, UNSUPPORTED_FORMAT, CANCELED }
	abstract fun ApplyItem(item: Item, progress: FileSelector.Progress): ApplyResult
}
