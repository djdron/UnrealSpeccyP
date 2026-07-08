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
import android.graphics.PorterDuff
import android.opengl.GLES20
import android.opengl.GLUtils
import android.os.SystemClock
import android.text.TextUtils

import app.usp.Emulator

class ControlOverlayReplay(context: Context) : ControlOverlay() {

	private val size: Int
	private val textures: IntArray = IntArray(2)
	private val pos_fill: Bitmap
	private val pos_dot: Bitmap
	private val HIDE_TIME_MS = 4000

	private val time_current: TimeLabel
	private val time_total: TimeLabel

	private var sprite: Int = -1

	open inner class TextLabel(context: Context, text_size_dp: Int, max_text: String) {
		var paint: Paint = Paint()
		private val bmp: Bitmap
		private val labelTextures: IntArray = IntArray(1)
		var width: Int = 0
		var height: Int = 0
		protected var text: String = ""
		private var labelSprite: Int = -1

		init {
			paint.isAntiAlias = true
			// 10 dp font size
			val textSize = (text_size_dp * context.resources.displayMetrics.density + 0.5f).toInt()
			paint.textSize = textSize.toFloat()
			paint.textAlign = Paint.Align.CENTER
			width = paint.measureText(max_text).toInt() + 5
			height = textSize + 5
			val width_pot = NextPot(width)
			val height_pot = NextPot(height)
			bmp = Bitmap.createBitmap(width_pot, height_pot, Bitmap.Config.ARGB_8888)
		}

		fun Init() {
			GLES20.glGenTextures(1, labelTextures, 0)
			GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, labelTextures[0])
			GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0)
			labelSprite = Emulator.the.GLSpriteCreate(width, height)
		}

		open fun Draw(_text: String, x: Int, y: Int, alpha: Float) {
			if (TextUtils.isEmpty(_text)) return

			if (text != _text) {
				text = _text
				val canvas = Canvas(bmp)
				canvas.clipRect(0, 0, width, height)
				canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
				paint.color = Color.BLACK
				canvas.drawText(text, (width/2 + 3).toFloat(), (height - 4).toFloat(), paint)
				paint.color = Color.WHITE
				canvas.drawText(text, (width/2 + 2).toFloat(), (height - 5).toFloat(), paint)
				GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, labelTextures[0])
				GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0)
			}
			Emulator.the.GLSpriteDraw(labelSprite, labelTextures[0], x, y, width, height, alpha * 0.998f, false)
		}
	}

	inner class TimeLabel(context: Context, text_size_dp: Int) : TextLabel(context, text_size_dp, "88:88:88") {
		private var seconds: Int = -1

		fun Draw(_seconds: Int, x: Int, y: Int, alpha: Float) {
			if (seconds == _seconds) {
				super.Draw(this.text, x, y, alpha)
				return
			}
			seconds = _seconds
			val h = seconds / 3600
			val m = (seconds % 3600) / 60
			val s = seconds % 60

			val time = if (h > 0) {
				String.format("%02d:%02d:%02d", h, m, s)
			} else {
				String.format("%02d:%02d", m, s)
			}
			super.Draw(time, x, y, alpha)
		}
	}

	init {
		size = (context.resources.displayMetrics.density*9).toInt()
		val size_pot = NextPot(size)

		val paint = Paint()
		paint.isAntiAlias = true

		pos_fill = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888)
		var canvas = Canvas(pos_fill)
		paint.color = Color.WHITE
		canvas.drawRect(0f, (size*2/9).toFloat(), size.toFloat(), (size*7/9).toFloat(), paint)

		pos_dot = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888)
		canvas = Canvas(pos_dot)
		paint.color = Color.WHITE
		canvas.drawCircle(size*0.5f, size*0.5f, size*0.5f, paint)

		time_current = TimeLabel(context, 12)
		time_total = TimeLabel(context, 12)
	}

	fun Init() {
		GLES20.glGenTextures(2, textures, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, pos_fill, 0)
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[1])
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, pos_dot, 0)
		time_current.Init()
		time_total.Init()
		sprite = Emulator.the.GLSpriteCreate(size, size)
	}

	fun OnTouch(x: Float, y: Float, down: Boolean, pointer_id: Int) {
		if (!active) return
		touch_time = SystemClock.uptimeMillis()
	}

	fun Draw(_w: Int, _h: Int) {
		if (!active) return

		val passed_time = SystemClock.uptimeMillis() - touch_time
		if (passed_time > HIDE_TIME_MS) return

		val progress = Emulator.the.getReplayProgress() ?: return

		val alpha = if (passed_time > HIDE_TIME_MS - 1000) {
			(HIDE_TIME_MS - passed_time).toFloat() / 1000.0f
		} else {
			1.0f
		}

		val px = (time_current.width * 1.2f).toInt()
		val py = (_h * 0.05f).toInt()
		val p = progress.frame_current.toFloat() / progress.frames_total
		val width = _w - px * 2
		val psx = (width * p).toInt()

		// draw progress
		Emulator.the.GLSpriteSetColor(sprite, 0.5f, 0.0f, 0.0f)
		Emulator.the.GLSpriteDraw(sprite, textures[0], px, py, psx, size, 0.8f * alpha, false)

		var psxc = psx
		if (progress.frame_current + progress.frames_cached < progress.frames_total) {
			val pc = (progress.frame_current + progress.frames_cached).toFloat() / progress.frames_total
			psxc = (width * pc).toInt()
			Emulator.the.GLSpriteSetColor(sprite, 0.4f, 0.4f, 0.4f)
			Emulator.the.GLSpriteDraw(sprite, textures[0], px + psx, py, psxc - psx, size, 0.8f * alpha, false)
		}

		Emulator.the.GLSpriteSetColor(sprite, 0.2f, 0.2f, 0.2f)
		Emulator.the.GLSpriteDraw(sprite, textures[0], px + psxc, py, width - psxc, size, 0.8f * alpha, false)

		Emulator.the.GLSpriteSetColor(sprite, 1.0f, 1.0f, 1.0f)
		Emulator.the.GLSpriteDraw(sprite, textures[1], px + psx - size / 2, py, size, size, 0.8f * alpha, false)

		time_current.Draw(progress.frame_current/50, (time_current.width*0.05f).toInt(), py + size/2 - time_current.height/2, 1.0f*alpha)
		time_total.Draw(progress.frames_total/50, _w - (time_current.width*1.05f).toInt(), py + size/2 - time_current.height/2, 1.0f*alpha)
	}
}
