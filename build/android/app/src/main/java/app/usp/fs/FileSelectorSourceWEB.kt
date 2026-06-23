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

import app.usp.Emulator
import java.io.File
import java.io.FileOutputStream
import java.net.URL
import java.net.URLConnection
import java.nio.ByteBuffer
import java.nio.charset.Charset

abstract class FileSelectorSourceWEB : FileSelectorSource() {

	abstract fun Root(): String?
	abstract fun FullURL(url: String): String
	abstract fun TextEncoding(): String
	abstract fun Items(): Array<String>
	abstract fun ItemsURLs(): Array<String>

	override fun GetItems(path: File, items: MutableList<Item>, progress: FileSelector.Progress): GetItemsResult {
		val pathUp = path.parentFile
		val root = Root()
		if (root == null) {
			if (pathUp == null) {
				for (i in Items()) {
					items.add(Item(this, i))
				}
				return GetItemsResult.OK
			}
			items.add(Item(this, "/.."))
		} else {
			if (pathUp == null) {
				items.add(Item(this, root))
				return GetItemsResult.OK
			}

			var r = path
			while (r.parentFile?.parentFile != null) {
				r = r.parentFile ?: break
			}

			if (r.toString() != root) return GetItemsResult.OK
			items.add(Item(this, "/.."))

			if (pathUp.parent == null) {
				if (Items().isEmpty()) {
					return ParseURL("", items, "", progress)
				} else {
					for (i in Items()) {
						items.add(Item(this, i))
					}
				}
				return GetItemsResult.OK
			}
		}
		var idx = 0
		val n = "/" + path.name
		for (i in Items()) {
			if (i == n) {
				return ParseURL(ItemsURLs()[idx], items, n, progress)
			}
			++idx
		}
		return GetItemsResult.FAIL
	}

	protected fun ParseURL(url: String, items: MutableList<Item>, name: String, progress: FileSelector.Progress): GetItemsResult {
		val s = LoadText(FullURL(url), TextEncoding(), progress) ?: return GetItemsResult.UNABLE_CONNECT
		if (progress.Canceled()) return GetItemsResult.CANCELED
		return ParseText(s, items, name, progress)
	}

	abstract fun ParseText(text: String, items: MutableList<Item>, name: String, progress: FileSelector.Progress): GetItemsResult

	protected fun LoadFile(url: String, name: File, progress: FileSelector.Progress): Boolean {
		try {
			val path = name.parentFile
			path?.mkdirs()
			val fileTmp = File(name.path + ".tmp")

			FileOutputStream(fileTmp).use { os ->
				val connection = URL(url).openConnection()
				connection.inputStream.use { `is` ->
					val len = connection.contentLength
					val buffer = ByteArray(256 * 1024)
					var size = 0
					var r: Int

					while (`is`.read(buffer).also { r = it } != -1) {
						os.write(buffer, 0, r)
						size += r
						if (progress.Canceled()) break
						if (len > 0) progress.OnProgress(size, len)
					}
				}
			}

			return if (!progress.Canceled()) {
				fileTmp.renameTo(name)
			} else {
				fileTmp.delete()
				true
			}
		} catch (e: Exception) {
		}
		return false
	}

	protected open fun SetupConnection(connection: URLConnection) {}

	protected fun LoadText(_url: String, _encoding: String, progress: FileSelector.Progress): String? {
		try {
			val connection = URL(_url).openConnection()
			SetupConnection(connection)

			connection.inputStream.use { `is` ->
				val len = connection.contentLength
				val buffer = ByteArray(16384)
				var buf = ByteBuffer.allocate(0)
				var r: Int

				while (`is`.read(buffer).also { r = it } != -1) {
					val buf1 = ByteBuffer.allocate(buf.capacity() + r)
					buf1.put(buf)
					buf1.put(buffer, 0, r)
					buf = buf1
					buf.rewind()

					if (progress.Canceled()) return ""
					if (len > 0) progress.OnProgress(buf.capacity(), len)
				}

				val charset = Charset.forName(_encoding)
				val decoder = charset.newDecoder()
				return decoder.decode(buf).toString()
			}
		} catch (e: Exception) {
		}
		return null
	}

	protected fun OpenFile(url: String, name: File, progress: FileSelector.Progress): ApplyResult {
		if (!LoadFile(url, name, progress)) return ApplyResult.UNABLE_CONNECT2
		if (progress.Canceled()) return ApplyResult.CANCELED
		return if (Emulator.the.Open(name.toString())) ApplyResult.OK else ApplyResult.UNSUPPORTED_FORMAT
	}
}
