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

package app.usp

import java.io.File
import java.io.FileNotFoundException
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.nio.ByteBuffer

import android.app.AlertDialog
import android.app.UiModeManager
import android.content.Context
import android.content.res.Configuration
import android.database.Cursor
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.provider.MediaStore
import android.widget.PopupMenu
import android.widget.Toast
import android.view.Menu
import android.view.MenuItem
import android.content.Intent
import androidx.activity.ComponentActivity
import androidx.activity.OnBackPressedCallback
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.result.ActivityResultLauncher
import androidx.constraintlayout.widget.Guideline
import androidx.core.view.ViewCompat
import androidx.core.view.WindowCompat
import androidx.core.view.WindowInsetsCompat
import androidx.core.view.WindowInsetsControllerCompat
import androidx.preference.PreferenceManager
import app.usp.ctl.Control

open class Main : ComponentActivity() {

	private lateinit var view: ViewGLES
	private lateinit var control: Control
	private var hide_callback: Handler? = null
	private var hide_runnable: Runnable? = null
	private var paused: Boolean = false

	private val requestPermissionLauncher: ActivityResultLauncher<String> =
		registerForActivityResult(ActivityResultContracts.RequestPermission()) { isGranted ->
			if (isGranted) {
				Open()
			} else {
				OnOpenFailed()
			}
		}

	override fun onCreate(savedInstanceState: Bundle?) {
		val theme = PreferenceManager.getDefaultSharedPreferences(this).getString("theme", "dark")
		val uiModeManager = getSystemService(Context.UI_MODE_SERVICE) as UiModeManager

		val mode = when (theme) {
			"dark" -> UiModeManager.MODE_NIGHT_YES
			"light" -> UiModeManager.MODE_NIGHT_NO
			else -> UiModeManager.MODE_NIGHT_AUTO
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) { // Android 12+
			uiModeManager.setApplicationNightMode(mode)
		} else {
			uiModeManager.setNightMode(mode)
		}

		super.onCreate(savedInstanceState)

		if (Emulator.the == null) {
			Emulator.the = Emulator()
			Emulator.the.InitRom(0, BinRes(R.raw.sos128_0))
			Emulator.the.InitRom(1, BinRes(R.raw.sos128_1))
			Emulator.the.InitRom(2, BinRes(R.raw.sos48))
			Emulator.the.InitRom(3, BinRes(R.raw.service))
			Emulator.the.InitRom(4, BinRes(R.raw.dos513f))
			Emulator.the.InitFont(BinRes(R.raw.spxtrm4f))
			Emulator.the.Init(filesDir.toString())
		}

		setContentView(R.layout.main)
		view = findViewById(R.id.view_gles)
		control = findViewById(R.id.control)

		if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			val guideline = findViewById<Guideline>(R.id.control_guideline)
			guideline.setGuidelinePercent(1.0f)
		}

		view.open_menu = Runnable { OpenMenu() }

		onBackPressedDispatcher.addCallback(this, object : OnBackPressedCallback(true) {
			override fun handleOnBackPressed() {
				OpenMenu()
			}
		})

		ViewCompat.setOnApplyWindowInsetsListener(window.decorView) { _, insets ->
			if (!paused && insets.isVisible(WindowInsetsCompat.Type.systemBars())) {
				RunHideCallback()
			}
			insets
		}

		view.requestFocus()
		view.keepScreenOn = true
		Open()
	}

	override fun onConfigurationChanged(newConfig: Configuration) {
		super.onConfigurationChanged(newConfig)
		val guideline = findViewById<Guideline>(R.id.control_guideline)
		val percent = if (newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE) 1.0f else 0.6f
		guideline.setGuidelinePercent(percent)
	}

	private fun getFileName(uri: Uri): String? {
		if (uri.scheme == "content") {
			val cursor: Cursor? = contentResolver.query(
				uri,
				arrayOf(MediaStore.Images.Media.DATA),
				null,
				null,
				null
			)

			cursor?.use { c ->
				try {
					if (c.moveToFirst()) {
						val pathIndex = c.getColumnIndex(MediaStore.Images.Media.DATA)
						if (pathIndex != -1) {
							val pathString = c.getString(pathIndex)
							if (pathString != null) {
								return File(pathString).name
							}
						}
					}
				} catch (e: Exception) {
				}
			}
		}
		return uri.lastPathSegment
	}

	private fun Open(uri: Uri) {
		try {
			val path = File(filesDir.toString() + "/content/")
			path.mkdirs()

			val fileName = getFileName(uri)
			val file = File(path.path + "/" + fileName)
			val file_tmp = File(file.path + ".tmp")

			contentResolver.openInputStream(uri).use { `is` ->
				if (`is` == null) throw FileNotFoundException()

				FileOutputStream(file_tmp).use { os ->
					val buffer = ByteArray(256 * 1024)
					var r: Int
					var size = 0

					while (`is`.read(buffer).also { r = it } != -1) {
						os.write(buffer, 0, r)
						size += r
					}
				}
			}

			if (file_tmp.renameTo(file) && Emulator.the.Open(file.path)) {
				Toast.makeText(this, String.format(getString(R.string.opening), file), Toast.LENGTH_LONG).show()
			} else {
				Toast.makeText(this, String.format(getString(R.string.unable_open), file), Toast.LENGTH_LONG).show()
			}

		} catch (e: FileNotFoundException) {
			Toast.makeText(this, String.format(getString(R.string.unable_open), uri.toString()), Toast.LENGTH_LONG).show()
		} catch (e: IOException) {
			Toast.makeText(this, String.format(getString(R.string.unable_open), uri.toString()), Toast.LENGTH_LONG).show()
		}
	}

	private fun Open() {
		val intent = intent
		val uri = intent.data ?: return

		if (uri.scheme != "file") {
			Open(uri)
			return
		}

		val file = uri.path
		if (file.isNullOrEmpty()) return

		if (checkSelfPermission(android.Manifest.permission.WRITE_EXTERNAL_STORAGE) == android.content.pm.PackageManager.PERMISSION_GRANTED) {
			if (Emulator.the.Open(file)) {
				Toast.makeText(this, String.format(getString(R.string.opening), file), Toast.LENGTH_LONG).show()
			} else {
				Toast.makeText(this, String.format(getString(R.string.unable_open), file), Toast.LENGTH_LONG).show()
			}
		} else {
			if (shouldShowRequestPermissionRationale(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
				val dlg = AlertDialog.Builder(this)
				dlg.setMessage(getString(R.string.need_storage_permission))
				dlg.setCancelable(false)
				dlg.setPositiveButton(getString(R.string.ok)) { _, _ ->
					requestPermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
				}
				val ad = dlg.create()
				ad.show()
			} else {
				requestPermissionLauncher.launch(android.Manifest.permission.WRITE_EXTERNAL_STORAGE)
			}
		}
	}

	private fun OnOpenFailed() {
		val dlg = AlertDialog.Builder(this)
		dlg.setMessage(getString(R.string.unable_access_storage))
		dlg.setCancelable(true)
		dlg.setPositiveButton(getString(R.string.preferences)) { _, _ ->
			val intent = Intent(
				android.provider.Settings.ACTION_APPLICATION_DETAILS_SETTINGS,
				Uri.fromParts("package", packageName, null)
			)
			startActivity(intent)
		}
		dlg.setNegativeButton(getString(R.string.cancel), null)

		val ad = dlg.create()
		ad.show()
	}

	private fun RunHideCallback() {
		CancelHideCallback()

		hide_callback = Handler(mainLooper)
		hide_runnable = Runnable { HideSystemUI() }

		hide_callback?.postDelayed(hide_runnable!!, 2000)
	}

	private fun CancelHideCallback() {
		val runnable = hide_runnable ?: return

		hide_callback?.removeCallbacks(runnable)
		hide_callback = null
		hide_runnable = null
	}

	private fun HideSystemUI() {
		CancelHideCallback()
		EndPause()

		val controller = WindowInsetsControllerCompat(window, window.decorView)
		WindowCompat.setDecorFitsSystemWindows(window, false)
		controller.hide(WindowInsetsCompat.Type.systemBars())
		controller.systemBarsBehavior = WindowInsetsControllerCompat.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
	}

	override fun onWindowFocusChanged(hasFocus: Boolean) {
		super.onWindowFocusChanged(hasFocus)
		if (hasFocus) {
			HideSystemUI()
		}
	}

	override fun onDestroy() {
		CancelHideCallback()
		EndPause()
		super.onDestroy()
	}

	override fun onResume() {
		super.onResume()
		view.OnActivityResume()
		EndPause()
	}

	override fun onPause() {
		Emulator.the.StoreOptions()
		view.OnActivityPause()
		super.onPause()
	}

	private fun BeginPause() {
		if (!paused) {
			paused = true
			Emulator.the.VideoPaused(true)
		}
	}

	private fun EndPause() {
		if (paused) {
			paused = false
			Emulator.the.VideoPaused(false)
		}
	}

	private fun OpenMenu() {
		CancelHideCallback()
		BeginPause()
		val m = PopupMenu(this, control)
		m.inflate(R.menu.menu)
		m.show()
		m.setOnMenuItemClickListener { item -> OnMenuItemSelected(item) }
	}

	override fun onCreateOptionsMenu(menu: Menu): Boolean {
		menuInflater.inflate(R.menu.menu, menu)
		return super.onCreateOptionsMenu(menu)
	}

	override fun onPrepareOptionsMenu(menu: Menu): Boolean {
		CancelHideCallback()
		BeginPause()
		return super.onPrepareOptionsMenu(menu)
	}

	override fun onOptionsMenuClosed(menu: Menu) {
		EndPause()
		super.onOptionsMenuClosed(menu)
	}

	override fun onOptionsItemSelected(item: MenuItem): Boolean {
		if (OnMenuItemSelected(item)) {
			return true
		}
		return super.onOptionsItemSelected(item)
	}

	private fun OnMenuItemSelected(item: MenuItem): Boolean {

		EndPause()
		return when (item.itemId) {
			R.id.open_file -> {
				startActivity(Intent(this, FileOpen::class.java))
				true
			}
			R.id.save_state -> {
				Emulator.the.SaveState()
				true
			}
			R.id.load_state -> {
				Emulator.the.LoadState()
				true
			}
			R.id.toggle_control -> {
				control.OnToggle()
				view.UpdateControls()
				true
			}
			R.id.reset -> {
				Emulator.the.Reset()
				true
			}
			R.id.preferences -> {
				startActivity(Intent(this, Preferences::class.java))
				true
			}
			R.id.quit -> {
				Exit()
				true
			}
			else -> false
		}
	}

	private fun Exit() {
		finish()
	}

	private fun BinRes(id: Int): ByteBuffer {
		val `is` = resources.openRawResource(id)
		var data = ByteArray(0)
		try {
			data = ByteArray(`is`.available())
			`is`.read(data)
		} catch (e: IOException) {
		} finally {
			try { `is`.close() } catch (e: IOException) {}
		}
		val bb = ByteBuffer.allocateDirect(data.size)
		bb.put(data)
		bb.rewind()
		return bb
	}
}
