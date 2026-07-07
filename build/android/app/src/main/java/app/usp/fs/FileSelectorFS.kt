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
import java.util.zip.ZipFile
import android.os.Bundle
import app.usp.Emulator

class FileSelectorFS : FileSelector() {

	companion object {
		private val state = State()
	}

	override fun getState(): State {
		return state
	}

	override fun LongUpdate(path: File): Boolean {
		return false
	}

	override fun LongUpdateTitle(): Int {
		return 0
	}

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		sources.add(FileSelectorSourceFS_Cache())
		sources.add(FileSelectorSourceFS_ExternalStorage())
	}

	override fun onResume() {
		if (!update_on_resume) {
			super.onResume()
			return
		}

		getItems().clear()
		val lastFile = Emulator.the.GetLastFile()
		var fss: FileSelectorSourceFS? = null

		for (s in sources) {
			val fs = s as FileSelectorSourceFS
			val r = fs.RootPath().path
			if (lastFile.startsWith(r)) {
				val lastFile2 = File(lastFile)
				if (lastFile2.exists() && lastFile2.canRead()) {
					fss = fs
					break
				}
				val zipPath = fs.ZipPath(lastFile2.parentFile)
				if (zipPath != null) {
					fss = fs
					break
				}
			}
		}

		if (fss != null) {
			val r = fss.RootPath().path
			val lastFileLocal = File(fss.Root() + "/" + lastFile.substring(r.length))
			getState().last_name = lastFileLocal.name
			getState().current_path = lastFileLocal.parentFile ?: File("/")
		} else {
			getState().current_path = File("/")
			getState().last_name = ""
		}

		super.onResume()
	}

	fun UpdateAll() {
		getItems().clear()
		update_on_resume = false
		Update()
	}

	fun ResetToRoot() {
		getItems().clear()
		getState().current_path = File("/")
		getState().last_name = ""
		update_on_resume = false
		Update()
	}

	abstract inner class FileSelectorSourceFS : FileSelectorSource() {
		abstract fun Root(): String
		abstract fun RootPath(): File

		private val ZIP_EXT = "zip"

		private fun IsZipName(name: String): Boolean {
			return name.substringAfterLast('.', "").lowercase() == ZIP_EXT
		}

		fun ZipPath(path: File?): File? {
			var zipPath = path
			while (zipPath != null) {
				if (zipPath.canRead() && !zipPath.isDirectory && IsZipName(zipPath.toString())) {
					break
				}
				zipPath = zipPath.parentFile
			}
			return zipPath
		}

		override fun GetItems(path: File, items: MutableList<Item>, progress: Progress): GetItemsResult {
			val pathUp = path.parentFile
			if (pathUp == null) {
				items.add(Item(this, Root()))
				return GetItemsResult.OK
			}

			val p = path.path
			if (!p.startsWith(Root())) return GetItemsResult.OK

			items.add(Item(this, "/.."))
			val path = File(RootPath().path + "/" + p.substring(Root().length))

			val zipPath = ZipPath(path)
			if (zipPath != null) {
				try {
					ZipFile(zipPath, ZipFile.OPEN_READ).use { zif ->
						val entries = zif.entries()
						while (entries.hasMoreElements()) {
							val ze = entries.nextElement()
							val zef = File(zipPath.path + "/" + ze.name)
							val zefOk = zef.parentFile
							if (zefOk == path) {
								val zname = File(ze.name)
								if (ze.isDirectory) {
									items.add(Item(this, "/" + zname.name))
								} else if (Emulator.the.FileTypeSupported(zname.name)) {
									items.add(Item(this, zname.name))
								}
							}
							if (progress.Canceled()) return GetItemsResult.CANCELED
						}
					}
				} catch (e: Exception) {
				}
			} else if (path.canRead()) {
				val files = path.listFiles()
				if (files != null) {
					for (f in files) {
						val name = f.name
						if (f.isDirectory || IsZipName(name)) {
							items.add(Item(this, "/$name"))
						} else if (Emulator.the.FileTypeSupported(name)) {
							items.add(Item(this, name))
						}
					}
				}
			}

			items.sortWith(Comparator { itemA, itemB ->
				val a = itemA.name
				val b = itemB.name
				if (a.isEmpty() || b.isEmpty()) return@Comparator a.compareTo(b, ignoreCase = true)

				val aDir = a.startsWith("/")
				val bDir = b.startsWith("/")

				if (aDir != bDir) {
					return@Comparator if (aDir) -1 else 1
				}
				a.compareTo(b, ignoreCase = true)
			})

			return GetItemsResult.OK
		}

		override fun ApplyItem(item: Item, progress: Progress): ApplyResult {
			val f = File(getState().current_path.path + "/" + item.name)
			val n = File(RootPath().path + "/" + f.path.substring(Root().length))
			return if (Emulator.the.Open(n.toString())) ApplyResult.OK else ApplyResult.UNSUPPORTED_FORMAT
		}
	}

	inner class FileSelectorSourceFS_Cache : FileSelectorSourceFS() {
		override fun RootPath(): File = context?.filesDir ?: File("")
		override fun Root(): String = "/cache"
	}

	inner class FileSelectorSourceFS_ExternalStorage : FileSelectorSourceFS() {
		override fun RootPath(): File = context?.getExternalFilesDir(null) ?: File("")
		override fun Root(): String = "/sdcard"
	}
}
