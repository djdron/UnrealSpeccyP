/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2012 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.opengl.GLUtils;
import android.os.SystemClock;
import app.usp.Emulator;
import app.usp.ViewGLES;

public class ControlController
{
	private final int size;
	private final int ptr_size;
	private boolean active = false;
	private long touch_time = 0;
	private float dir_x = 0.0f;
	private float dir_y = 0.0f;
	private int pid_joy = -1;

	private int[] textures = new int[2];
	private Bitmap joy_area = null;
	private Bitmap joy_ptr = null;

	public ControlController(int _size)
	{
		size = _size;
		ptr_size = (int)(size*0.4f);

	    Paint paint = new Paint();
	    paint.setAntiAlias(true);

        joy_area = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
	    Canvas canvas = new Canvas(joy_area);
        paint.setColor(Color.GRAY);
	    canvas.drawCircle(size*0.5f, size*0.5f, size*0.45f, paint);

        joy_ptr = Bitmap.createBitmap(size, size, Bitmap.Config.ARGB_8888);
	    canvas = new Canvas(joy_ptr);
        paint.setColor(Color.GRAY);
	    canvas.drawCircle(size*0.5f, size*0.5f, ptr_size*0.5f, paint);
	}
	public void Init(GL10 gl)
	{
		gl.glGenTextures(2, textures, 0);
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
	    GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, joy_area, 0);
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[1]);
	    GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, joy_ptr, 0);
	}
	public void OnTouch(float x, float y, boolean down, int pointer_id)
	{
		if(!active)
			return;
		touch_time = SystemClock.uptimeMillis();
		if(down && x < size*1.3f && y < size*1.3f)
		{
			pid_joy = pointer_id;
			float dx = x - size/2;
			float dy = y - size/2;
			final float dir_len = (float)Math.sqrt(dx*dx + dy*dy);
			float scale = dir_len / (size/2);
			if(scale > 1.0f)
				scale = 1.0f;
			dx *= scale/dir_len;
			dy *= scale/dir_len;
			Emulator.the.OnKey('r', dx > +0.3f, false, false);
			Emulator.the.OnKey('l', dx < -0.3f, false, false);
			Emulator.the.OnKey('u', dy > +0.3f, false, false);
			Emulator.the.OnKey('d', dy < -0.3f, false, false);
			dir_x = dx;
			dir_y = dy;
		}
		else if(pointer_id == pid_joy)
		{
			pid_joy = -1;
			Emulator.the.OnKey('r', false, false, false);
			Emulator.the.OnKey('l', false, false, false);
			Emulator.the.OnKey('u', false, false, false);
			Emulator.the.OnKey('d', false, false, false);
		}
		else
		{
			Emulator.the.OnKey('f', down, false, false);
		}
	}
	public void Draw(GL10 gl, ViewGLES.Quad quad)
	{
		if(!active)
			return;
		final long passed_time = SystemClock.uptimeMillis() - touch_time;
		if(passed_time > 2000)
		{
			dir_x = 0;
			dir_y = 0;
			return;
		}
		if(pid_joy < 0 && passed_time > 30)
		{
			dir_x *= 0.5f;
			dir_y *= 0.5f;
		}

		final float alpha = passed_time > 1000 ? (float)(2000 - passed_time)/1000.0f : 1.0f;

		// draw controller
		gl.glColor4f(1.0f, 1.0f, 1.0f, 0.3f*alpha);
	    gl.glViewport(0, 0, size, size);
		gl.glMatrixMode(GL10.GL_TEXTURE);
	    gl.glLoadIdentity();
	    gl.glMatrixMode(GL10.GL_MODELVIEW);
		gl.glLoadIdentity();
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
		gl.glEnable(GL10.GL_BLEND);
		gl.glBlendFunc(GL10.GL_SRC_ALPHA, GL10.GL_ONE_MINUS_SRC_ALPHA);
		quad.Draw(gl);

		gl.glTranslatef(dir_x*0.25f, -dir_y*0.25f, 0.0f);
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[1]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
		quad.Draw(gl);
	}
	public void Active(boolean on)
	{
		if(active && !on)
		{
			dir_x = 0;
			dir_y = 0;
			Emulator.the.OnKey('r', false, false, false);
			Emulator.the.OnKey('l', false, false, false);
			Emulator.the.OnKey('u', false, false, false);
			Emulator.the.OnKey('d', false, false, false);
			Emulator.the.OnKey('f', false, false, false);
			pid_joy = -1;
		}
		else if(!active && on)
		{
			if(touch_time == 0)
				touch_time = SystemClock.uptimeMillis();
		}
		active = on;
	}
}
