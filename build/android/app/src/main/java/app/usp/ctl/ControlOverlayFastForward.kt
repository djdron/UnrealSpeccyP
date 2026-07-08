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

import android.graphics.Bitmap
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Path
import android.opengl.GLES20
import android.opengl.GLUtils
import android.os.SystemClock

import app.usp.Emulator

class ControlOverlayFastForward(context: Context) : ControlOverlay() {

	private val size: Int
	private val textures: IntArray = IntArray(1)
	private val arrows: Bitmap
	private val HIDE_TIME_MS = 2000
	private var sprite: Int = -1

	private var pressed: Boolean = false

	init {
		size = (context.resources.displayMetrics.density*50).toInt()
		val size_pot = NextPot(size)

		val paint = Paint()
		paint.isAntiAlias = true

		arrows = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888)
		val canvas = Canvas(arrows)
		paint.color = Color.GRAY
		paint.style = Paint.Style.FILL

		val path = Path()
		path.moveTo(size / 8f, size / 4f)
		path.rLineTo(0f, size / 2f)
		path.rLineTo(size / 4f, -size / 4f)
		path.rLineTo(-size / 4f, -size / 4f)
		canvas.drawPath(path, paint)

		path.reset()
		path.moveTo(size / 2f, size / 4f)
		path.rLineTo(0f, size / 2f)
		path.rLineTo(size / 4f, -size / 4f)
		path.rLineTo(-size / 4f, -size / 4f)
		canvas.drawPath(path, paint)
	}

	fun Init() {
		GLES20.glGenTextures(1, textures, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, arrows, 0)

		sprite = Emulator.the.GLSpriteCreate(size, size)
	}

	fun OnTouch(x: Float, y: Float, down: Boolean, pointer_id: Int): Boolean {
		if (!active) return false

		return if (x < size && y < size) {
			touch_time = SystemClock.uptimeMillis()
			pressed = down
			true
		} else {
			pressed = false
			false
		}
	}

	fun Draw(_w: Int, _h: Int) {
		if (!active) return

		var alpha = 1.0f
		if (!pressed) {
			val passed_time = SystemClock.uptimeMillis() - touch_time
			if (passed_time > HIDE_TIME_MS) return

			alpha = if (passed_time > HIDE_TIME_MS - 1000) {
				(HIDE_TIME_MS - passed_time).toFloat() / 1000.0f
			} else {
				1.0f
			}
		}

		// draw arrows
		Emulator.the.GLSpriteDraw(sprite, textures[0], _w - size, _h - size, size, size, 0.3f * alpha, false)
	}

	fun Pressed(): Boolean {
		return active && pressed
	}

	override fun Active(on: Boolean) {
		super.Active(on)
		pressed = false
	}
}
