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

package app.usp.ctl

import android.content.Context
import android.content.res.Configuration
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Canvas
import android.graphics.Paint
import android.opengl.GLES20
import android.opengl.GLUtils
import android.os.SystemClock
import app.usp.Emulator
import app.usp.R

class ControlOverlayKeyboard(context: Context) : ControlOverlay() {

	private var width: Int = 0
	private var height: Int = 0
	private var pid_activator: Int = -1
	private val HIDE_TIME_MS = 4000
	private var sprite: Int = -1
	private var sprite_width: Int = 0
	private var sprite_height: Int = 0
	private val textures: IntArray = IntArray(1)
	private val keyboard: Bitmap

	init {
		//@note : width & height may vary at runtime a little ;-)
		if (context.resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			width = context.resources.displayMetrics.widthPixels
			height = context.resources.displayMetrics.heightPixels
		} else {
			width = context.resources.displayMetrics.heightPixels
			height = context.resources.displayMetrics.widthPixels
		}

		val options = BitmapFactory.Options()
		options.inScaled = false

		val bm = if (width > 480) {
			BitmapFactory.decodeResource(context.resources, R.drawable.keyboard_hq, options)
		} else {
			BitmapFactory.decodeResource(context.resources, R.drawable.keyboard, options)
		}

		bm.setDensity(Bitmap.DENSITY_NONE)
		sprite_width = bm.width
		sprite_height = bm.height

		val width_pot = NextPot(sprite_width)
		val height_pot = NextPot(sprite_height)

		keyboard = Bitmap.createBitmap(width_pot, height_pot, Bitmap.Config.RGB_565)
		val paint = Paint()
		paint.isAntiAlias = true
		val canvas = Canvas(keyboard)
		canvas.drawBitmap(bm, 0.0f, 0.0f, paint)
	}

	fun Init() {
		GLES20.glGenTextures(1, textures, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, keyboard, 0)

		sprite = Emulator.the.GLSpriteCreate(sprite_width, sprite_height)
	}

	fun OnTouch(x: Float, y: Float, down: Boolean, pointer_id: Int) {
		if (!active) return

		val passed_time = SystemClock.uptimeMillis() - touch_time
		touch_time = SystemClock.uptimeMillis()

		if (passed_time > HIDE_TIME_MS && down) {
			pid_activator = pointer_id
			return
		}

		if (pid_activator == pointer_id) {
			if (!down) pid_activator = -1
			return
		}

		Emulator.the.OnTouch(true, x/width, y/height, down, pointer_id)
	}

	fun Draw(_w: Int, _h: Int) {
		if (!active) return
		if (Emulator.the.ReplayActive()) return

		width = _w
		height = _h

		val passed_time = SystemClock.uptimeMillis() - touch_time
		if (passed_time > HIDE_TIME_MS) {
			return
		}

		val alpha = if (passed_time > HIDE_TIME_MS - 1000) {
			(HIDE_TIME_MS - passed_time).toFloat() / 1000.0f
		} else {
			1.0f
		}

		// draw keyboard
		Emulator.the.GLSpriteDraw(sprite, textures[0], 0, 0, width, height, 0.5f*alpha, true)
	}
}
