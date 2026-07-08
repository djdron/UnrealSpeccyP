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
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.opengl.GLES20
import android.opengl.GLUtils
import android.os.SystemClock
import app.usp.Emulator
import kotlin.math.sqrt

class ControlOverlayController(context: Context) : ControlOverlay() {

	private val size: Int
	private var touch_joy_time: Long = 0
	private var dir_x: Float = 0.0f
	private var dir_y: Float = 0.0f
	private var pid_joy: Int = -1
	private var sprite: Int = -1
	private val textures: IntArray = IntArray(2)
	private var joy_area: Bitmap
	private var joy_ptr: Bitmap

	init {
		size = (context.resources.displayMetrics.density*150).toInt()
		val ptr_size = (size*0.4f).toInt()
		val size_pot = NextPot(size)

		val paint = Paint()
		paint.isAntiAlias = true

		joy_area = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888)
		var canvas = Canvas(joy_area)
		paint.color = Color.GRAY
		canvas.drawCircle(size*0.5f, size*0.5f, size*0.45f, paint)

		joy_ptr = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888)
		canvas = Canvas(joy_ptr)
		paint.color = Color.GRAY
		canvas.drawCircle(size*0.5f, size*0.5f, ptr_size*0.5f, paint)
	}

	fun Init() {
		GLES20.glGenTextures(2, textures, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, joy_area, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[1])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, joy_ptr, 0)
		sprite = Emulator.the.GLSpriteCreate(size, size)
	}

	fun OnTouch(x: Float, y: Float, down: Boolean, pointer_id: Int) {
		if(!active)
			return
		touch_time = SystemClock.uptimeMillis()
		if (down && x < size*1.3f && y < size*1.3f) {
			touch_joy_time = touch_time
			pid_joy = pointer_id
			var dx = x - size/2
			var dy = y - size/2
			val dir_len = sqrt(dx*dx + dy*dy)
			var scale = dir_len / (size/2)
			if(scale > 1.0f) {
				scale = 1.0f
			}
			dx *= scale/dir_len
			dy *= scale/dir_len
			Control.UpdateJoystickKeys(dx < -0.3f, dx > +0.3f, dy > +0.3f, dy < -0.3f)
			dir_x = dx
			dir_y = dy
		} else if (pointer_id == pid_joy) {
			pid_joy = -1
			Emulator.the.OnKey('r', false, false, false)
			Emulator.the.OnKey('l', false, false, false)
			Emulator.the.OnKey('u', false, false, false)
			Emulator.the.OnKey('d', false, false, false)
		} else {
			Emulator.the.OnKey('f', down, false, false)
		}
	}

	fun Draw(width: Int) {
		if (!active) return
		if (Emulator.the.ReplayActive()) return

		val passed_time = SystemClock.uptimeMillis() - touch_time
		if (passed_time > 2000) {
			dir_x = 0f
			dir_y = 0f
			return
		}

		val passed_joy_time = SystemClock.uptimeMillis() - touch_joy_time
		if (pid_joy < 0 && passed_joy_time > 30) {
			dir_x *= 0.5f
			dir_y *= 0.5f
		}

		val alpha = if (passed_time > 1000) (2000 - passed_time).toFloat() / 1000.0f else 1.0f
		Emulator.the.GLSpriteDraw(sprite, textures[0], 0, 0, size, size, 0.3f * alpha, false)
		Emulator.the.GLSpriteDraw(sprite, textures[1], (dir_x*0.25f*size).toInt(), (dir_y*0.25f*size).toInt(), size, size, 0.3f*alpha, false)
		Emulator.the.GLSpriteDraw(sprite, textures[1], width - size, 0, size, size, 0.3f*alpha, false)
	}

	override fun Active(on: Boolean) {
		if (active && !on) {
			dir_x = 0f
			dir_y = 0f
			Emulator.the.OnKey('r', false, false, false)
			Emulator.the.OnKey('l', false, false, false)
			Emulator.the.OnKey('u', false, false, false)
			Emulator.the.OnKey('d', false, false, false)
			Emulator.the.OnKey('f', false, false, false)
			pid_joy = -1
		}
		super.Active(on)
	}
}
