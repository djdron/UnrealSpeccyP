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
import android.util.AttributeSet
import android.widget.ImageView
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import app.usp.Emulator
import app.usp.Preferences
import app.usp.R

class Control(context: Context, attrs: AttributeSet?) : ImageView(context, attrs) {

	private val keyboard: Bitmap
	private val joystick: Bitmap
	private var keyboardActive = false

	init {
		val orientation = resources.configuration.orientation
		val width = if (orientation == Configuration.ORIENTATION_PORTRAIT) {
			context.resources.displayMetrics.widthPixels
		} else {
			context.resources.displayMetrics.heightPixels
		}

		val options = BitmapFactory.Options().apply { inScaled = false }

		if (width > 480) {
			keyboard = BitmapFactory.decodeResource(resources, R.drawable.keyboard_hq, options)
			joystick = BitmapFactory.decodeResource(resources, R.drawable.joystick_hq, options)
		} else {
			keyboard = BitmapFactory.decodeResource(resources, R.drawable.keyboard, options)
			joystick = BitmapFactory.decodeResource(resources, R.drawable.joystick, options)
		}

		keyboardActive = Emulator.the.GetOptionBool(Preferences.use_keyboard_id)

		adjustViewBounds = true
		scaleType = ScaleType.FIT_XY
		setImageBitmap(if (keyboardActive) keyboard else joystick)

		setOnTouchListener(object : ControlTouch() {
			override fun OnTouch(x: Float, y: Float, down: Boolean, pid: Int) {
				Emulator.the.OnTouch(keyboardActive, x / width, y / height, down, pid)
			}
		})
	}

	override fun onMeasure(w: Int, h: Int) {
		if (resources.configuration.orientation == Configuration.ORIENTATION_LANDSCAPE) {
			setMeasuredDimension(0, 0)
		} else {
			super.onMeasure(w, h)
		}
	}

	fun OnToggle() {
		keyboardActive = !keyboardActive
		Emulator.the.SetOptionBool(Preferences.use_keyboard_id, keyboardActive)
		setImageBitmap(if (keyboardActive) keyboard else joystick)
	}

	companion object {
		@JvmStatic
		fun UpdateJoystickKeys(left: Boolean, right: Boolean, up: Boolean, down: Boolean) {
			val em = Emulator.the
			val keys = listOf('l', 'r', 'u', 'd')
			val states = listOf(left, right, up, down)

			// Release first
			keys.forEachIndexed { i, key -> if (!states[i]) em.OnKey(key.toChar(), false, false, false) }
			// Press then
			keys.forEachIndexed { i, key -> if (states[i]) em.OnKey(key.toChar(), true, false, false) }
		}
	}
}
