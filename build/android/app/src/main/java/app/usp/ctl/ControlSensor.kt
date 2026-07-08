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
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.hardware.display.DisplayManager
import android.view.Display
import android.view.Surface
import app.usp.Emulator
import app.usp.Preferences

class ControlSensor(context: Context) : SensorEventListener {

	private var accelerometer: Sensor? = null
	private var sensor_manager: SensorManager? = null
	private var display: Display? = null

	companion object {
		const val SENSOR_THRESHOLD = 1.0f
	}

	init {
		sensor_manager = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
		accelerometer = sensor_manager?.getDefaultSensor(Sensor.TYPE_ACCELEROMETER)

		display = if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
			context.display
		} else {
			val display_manager = context.getSystemService(Context.DISPLAY_SERVICE) as DisplayManager
			display_manager.getDisplay(Display.DEFAULT_DISPLAY)
		}
	}

	fun Install() {
		if (Emulator.the.GetOptionBool(Preferences.use_sensor_id)) {
			sensor_manager?.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_UI)
		}
	}

	fun Uninstall() {
		if (Emulator.the.GetOptionBool(Preferences.use_sensor_id)) {
			sensor_manager?.unregisterListener(this)
		}
	}

	override fun onAccuracyChanged(sensor: Sensor, accuracy: Int) {
	}

	override fun onSensorChanged(event: SensorEvent) {
		if (event.sensor.type != Sensor.TYPE_ACCELEROMETER) return

		var sx = 0f
		var sy = 0f

		val rotation = display?.rotation ?: Surface.ROTATION_0

		// Вернул правильные индексы массивов [0] и [1] для координат акселерометра
		when (rotation) {
			Surface.ROTATION_0 -> {
				sx = -event.values[0]
				sy = event.values[1]
			}
			Surface.ROTATION_90 -> {
				sx = event.values[1]
				sy = event.values[0]
			}
			Surface.ROTATION_180 -> {
				sx = event.values[0]
				sy = -event.values[1]
			}
			Surface.ROTATION_270 -> {
				sx = -event.values[1]
				sy = -event.values[0]
			}
		}

		Control.UpdateJoystickKeys(sx < -SENSOR_THRESHOLD, sx > +SENSOR_THRESHOLD, sy < -SENSOR_THRESHOLD, sy > +SENSOR_THRESHOLD)
	}
}
