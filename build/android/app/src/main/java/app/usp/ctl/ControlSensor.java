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

package app.usp.ctl;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.hardware.display.DisplayManager;
import android.view.Display;
import android.view.Surface;
import app.usp.Emulator;
import app.usp.Preferences;

public class ControlSensor implements SensorEventListener 
{
	static final float SENSOR_THRESHOLD = 1.0f;
	private Sensor accelerometer;
	private SensorManager sensor_manager;
	private Display display;
	public ControlSensor(Context context)
	{
		sensor_manager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
		accelerometer = sensor_manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
			display = context.getDisplay();
		} else {
			DisplayManager display_manager = (DisplayManager)context.getSystemService(Context.DISPLAY_SERVICE);
			display = display_manager.getDisplay(android.view.Display.DEFAULT_DISPLAY);
		}
	}
	public void Install()
	{
		if(Emulator.the.GetOptionBool(Preferences.use_sensor_id))
			sensor_manager.registerListener(this, accelerometer, SensorManager.SENSOR_DELAY_UI);
	}
	public void Uninstall()
	{
		if(Emulator.the.GetOptionBool(Preferences.use_sensor_id))
			sensor_manager.unregisterListener(this);
	}
	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy)
	{
	}
	@Override
	public void onSensorChanged(SensorEvent event)
	{
		if(event.sensor.getType() != Sensor.TYPE_ACCELEROMETER)
			return;
		float sx = 0, sy = 0; 
		switch(display.getRotation())
		{
		case Surface.ROTATION_0:
			sx = -event.values[0];
			sy = event.values[1];
			break;
		case Surface.ROTATION_90:
			sx = event.values[1];
			sy = event.values[0];
			break;
		case Surface.ROTATION_180:
			sx = event.values[0];
			sy = -event.values[1];
			break;
		case Surface.ROTATION_270:
			sx = -event.values[1];
			sy = -event.values[0];
			break;
		}
		Control.UpdateJoystickKeys(sx < -SENSOR_THRESHOLD, sx > +SENSOR_THRESHOLD, sy < -SENSOR_THRESHOLD, sy > +SENSOR_THRESHOLD);
	}
}
