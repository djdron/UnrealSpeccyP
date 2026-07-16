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

import android.content.Context
import android.content.res.Configuration
import android.opengl.GLSurfaceView
import android.os.Handler

import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

import android.util.AttributeSet
import android.view.KeyEvent
import android.widget.Toast

import app.usp.ctl.ControlKeys
import app.usp.ctl.ControlOverlayController
import app.usp.ctl.ControlOverlayFastForward
import app.usp.ctl.ControlOverlayKeyboard
import app.usp.ctl.ControlOverlayReplay
import app.usp.ctl.ControlSensor
import app.usp.ctl.ControlTouch

class ViewGLES(context: Context, attrs: AttributeSet?) : GLSurfaceView(context, attrs) {

	companion object {
		const val TIME_FRAME: Long = 20000000 // 20ms per frame - 50fps
	}

	private inner class Video(private val context: Context) : ControlTouch(), Renderer {
		val control_controller: ControlOverlayController = ControlOverlayController(context)
		val control_keyboard: ControlOverlayKeyboard = ControlOverlayKeyboard(context)
		val control_replay: ControlOverlayReplay = ControlOverlayReplay(context)
		val control_fast_forward: ControlOverlayFastForward = ControlOverlayFastForward(context)

		private var width = 0
		private var height = 0

		override fun OnTouch(x: Float, y: Float, down: Boolean, pid: Int) {
			control_replay.OnTouch(x, y, down, pid)
			if (control_fast_forward.OnTouch(width - x, y, down, pid)) return
			control_controller.OnTouch(x, height - y, down, pid)
			control_keyboard.OnTouch(x, y, down, pid)
		}

		override fun onSurfaceCreated(gl_unused: GL10?, config: EGLConfig?) {
			Emulator.the.GLDone()
			Emulator.the.GLInit()
			Emulator.the.ProfilerBegin(2)
			control_controller.Init()
			control_keyboard.Init()
			control_replay.Init()
			control_fast_forward.Init()
		}

		private fun ShowMessage(code: Int) {
			if (code > 0) {
				val h = Handler(context.mainLooper)
				h.post {
					val msg = when (code) {
						1 -> context.getString(R.string.rzx_finished)
						2 -> context.getString(R.string.rzx_sync_lost)
						3 -> context.getString(R.string.rzx_invalid)
						4 -> context.getString(R.string.rzx_unsupported)
						else -> null
					}
					if (msg != null) {
						Toast.makeText(context.applicationContext, msg, Toast.LENGTH_LONG).show()
					}
				}
			}
		}

		private fun Draw() {
			Emulator.the.ProfilerBegin(3)
			Emulator.the.GLDraw(width, height)
			Emulator.the.ProfilerEnd(3)

			Emulator.the.ProfilerBegin(1)
			control_controller.Draw(width)
			control_keyboard.Draw(width, height)
			control_replay.Draw(width, height)
			control_fast_forward.Draw(width, height)
			Emulator.the.ProfilerEnd(1)
		}

		override fun onDrawFrame(gl_unused: GL10?) {
			Emulator.the.ProfilerEnd(2)
			val time_frame_end = System.nanoTime() + TIME_FRAME*4/5
			ShowMessage(Emulator.the.Update())
			if (control_fast_forward.Pressed()) {
				for (frames in 1 until 10) { // do not speedup faster than 10x
					Emulator.the.AudioUpdate(true)
					ShowMessage(Emulator.the.Update())
					if (System.nanoTime() > time_frame_end) break
				}
			}
			Draw()
			Emulator.the.AudioUpdate(false)
			Emulator.the.ProfilerBegin(2)
		}

		override fun onSurfaceChanged(gl_unused: GL10?, w: Int, h: Int) {
			width = w
			height = h
		}
	}

	private val video: Video = Video(context)
	private val sensor: ControlSensor = ControlSensor(context)

	var open_menu: Runnable? = null

	init {
		setEGLContextClientVersion(2)
		setEGLConfigChooser(false)
		isFocusable = true
		isFocusableInTouchMode = true
		setOnKeyListener(ControlKeys())
		setRenderer(video)
		setOnTouchListener(video)
	}

	override fun onKeyDown(keyCode: Int, event: KeyEvent?): Boolean {
		when (keyCode) {
			KeyEvent.KEYCODE_F1,
			KeyEvent.KEYCODE_MENU,
			KeyEvent.KEYCODE_BUTTON_START -> return true
		}
		return super.onKeyDown(keyCode, event)
	}

	override fun onKeyUp(keyCode: Int, event: KeyEvent?): Boolean {
		when (keyCode) {
			KeyEvent.KEYCODE_F1,
			KeyEvent.KEYCODE_MENU,
			KeyEvent.KEYCODE_BUTTON_START -> {
				open_menu?.run()
				return true
			}
		}
		return super.onKeyUp(keyCode, event)
	}

	override fun onMeasure(w: Int, h: Int) {
		super.onMeasure(w, h)
		UpdateControls()
	}

	private fun InLandscapeMode(): Boolean {
		return resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE
	}

	fun OnActivityResume() {
		onResume()
		sensor.Install()
		Emulator.the.AudioInit();
	}

	fun OnActivityPause() {
		onPause()
		sensor.Uninstall()
		Emulator.the.AudioDone();
	}

	fun UpdateControls() {
		val a = InLandscapeMode()
		val k = Emulator.the.GetOptionBool(Preferences.use_keyboard_id)
		video.control_controller.Active(a && !k)
		video.control_keyboard.Active(a && k)
		video.control_replay.KickVisible()
		video.control_fast_forward.KickVisible()
	}
}
