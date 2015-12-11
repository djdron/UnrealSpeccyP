/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

import javax.microedition.khronos.opengles.GL10;

import android.graphics.Bitmap;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Path;
import android.opengl.GLUtils;
import android.os.SystemClock;

import app.usp.ViewGLES;

public class ControlFastForward extends ControlOverlay
{
	private final int size;
	private final float scale_pot;
	private int[] textures = new int[1];
	private Bitmap arrows = null;
	static final int HIDE_TIME_MS = 2000;

	private boolean pressed = false;

	public ControlFastForward(Context context)
	{
		size = (int)(context.getResources().getDisplayMetrics().density*50);
		final int size_pot = NextPot(size);
		scale_pot = ((float)size)/size_pot;

		Paint paint = new Paint();
		paint.setAntiAlias(true);

		arrows = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888);
		Canvas canvas = new Canvas(arrows);
		paint.setColor(Color.GRAY);
		paint.setStyle(Paint.Style.FILL);

		Path path = new Path();
		path.moveTo(size / 8, size / 4);
		path.rLineTo(0, size / 2);
		path.rLineTo(size / 4, -size / 4);
		path.rLineTo(-size / 4, -size / 4);
		canvas.drawPath(path, paint);

		path.reset();
		path.moveTo(size/2, size/4);
		path.rLineTo(0, size/2);
		path.rLineTo(size/4, -size/4);
		path.rLineTo(-size/4, -size/4);
		canvas.drawPath(path, paint);
	}
	public void Init(GL10 gl)
	{
		gl.glGenTextures(1, textures, 0);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, arrows, 0);
	}
	public boolean OnTouch(float x, float y, boolean down, int pointer_id)
	{
		if(!active)
			return false;
		if(x < size && y < size)
		{
			touch_time = SystemClock.uptimeMillis();
			pressed = down;
			return true;
		}
		else
		{
			pressed = false;
			return false;
		}
	}
	public void Draw(GL10 gl, ViewGLES.Quad quad, int _w, int _h)
	{
		if(!active)
			return;

		float alpha = 1.0f;
		if(!pressed)
		{
			final long passed_time = SystemClock.uptimeMillis() - touch_time;
			if(passed_time > HIDE_TIME_MS)
				return;
			alpha = passed_time > (HIDE_TIME_MS - 1000) ? (float) (HIDE_TIME_MS - passed_time) / 1000.0f : 1.0f;
		}

		// draw arrows
		gl.glColor4f(1.0f, 1.0f, 1.0f, 0.3f*alpha);
		gl.glViewport(_w - size, _h - size, size, size);
		gl.glMatrixMode(GL10.GL_TEXTURE);
		gl.glLoadIdentity();
		gl.glScalef(scale_pot, scale_pot, 1.0f);
		gl.glMatrixMode(GL10.GL_MODELVIEW);
		gl.glLoadIdentity();
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
		gl.glEnable(GL10.GL_BLEND);
		gl.glBlendFunc(GL10.GL_SRC_ALPHA, GL10.GL_ONE_MINUS_SRC_ALPHA);
		quad.Draw(gl);
	}
	public boolean Pressed() { return active && pressed; }
	public void Active(boolean on)
	{
		super.Active(on);
		pressed = false;
	}
}
