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

import android.content.Context;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.opengl.GLUtils;
import android.os.SystemClock;
import app.usp.Emulator;
import app.usp.R;
import app.usp.ViewGLES;

public class ControlKeyboard
{
	private final int width, height;
	private final float scale_x, scale_y;
	private boolean active = false;
	private long touch_time = 0;
	private int pid_activator = -1;
	static final int HIDE_TIME_MS = 4000; 

	private int[] textures = new int[1];
	private Bitmap keyboard = null;
	
	static int NextPot(int v)
	{
		--v;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		v |= (v >> 8);
		v |= (v >> 16);
		return ++v;
	}

	public ControlKeyboard(Context context)
	{
		if(context.getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE)
		{
			width = context.getResources().getDisplayMetrics().widthPixels;
			height = context.getResources().getDisplayMetrics().heightPixels;
		}
		else
		{
			width = context.getResources().getDisplayMetrics().heightPixels;
			height = context.getResources().getDisplayMetrics().widthPixels;
		}
		BitmapFactory.Options options = new BitmapFactory.Options();
		options.inScaled = false;
		Bitmap bm = BitmapFactory.decodeResource(context.getResources(), R.drawable.keyboard, options);
        bm.setDensity(Bitmap.DENSITY_NONE);
		final int w = bm.getWidth();
		final int h = bm.getHeight();
		final int width_pot = NextPot(w);
		final int height_pot = NextPot(h);
		scale_x = ((float)w)/width_pot;
		scale_y = ((float)h)/height_pot;
		
        keyboard = Bitmap.createBitmap(width_pot, height_pot, Bitmap.Config.ARGB_8888);
	    Paint paint = new Paint();
	    paint.setAntiAlias(true);
		Canvas canvas = new Canvas(keyboard);
		canvas.drawBitmap(bm, 0.0f, 0.0f, paint);
	}
	public void Init(GL10 gl)
	{
		gl.glGenTextures(1, textures, 0);
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
	    GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, keyboard, 0);
	}
	public void OnTouch(float x, float y, boolean down, int pointer_id)
	{
		if(!active)
			return;
		final long passed_time = SystemClock.uptimeMillis() - touch_time;
		touch_time = SystemClock.uptimeMillis();
		if(passed_time > HIDE_TIME_MS && down)
		{
			pid_activator = pointer_id;
			return;
		}
		if(pid_activator == pointer_id)
		{
			if(!down)
				pid_activator = -1;
			return;
		}
		Emulator.the.OnTouch(true, x/width, y/height, down, pointer_id);
	}
	public void Draw(GL10 gl, ViewGLES.Quad quad)
	{
		if(!active)
			return;
		final long passed_time = SystemClock.uptimeMillis() - touch_time;
		if(passed_time > HIDE_TIME_MS)
		{
			return;
		}

		final float alpha = passed_time > (HIDE_TIME_MS - 1000) ? (float)(HIDE_TIME_MS - passed_time)/1000.0f : 1.0f;

		// draw keyboard
		gl.glColor4f(1.0f, 1.0f, 1.0f, 0.8f*alpha);
	    gl.glViewport(0, 0, width, height);
		gl.glMatrixMode(GL10.GL_TEXTURE);
	    gl.glLoadIdentity();
	    gl.glScalef(scale_x, scale_y, 1.0f);
	    gl.glMatrixMode(GL10.GL_MODELVIEW);
		gl.glLoadIdentity();
	    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);
		gl.glEnable(GL10.GL_BLEND);
		gl.glBlendFunc(GL10.GL_SRC_ALPHA, GL10.GL_ONE_MINUS_SRC_ALPHA);
		quad.Draw();
	}
	public void Active(boolean on)
	{
		if(!active && on)
		{
			touch_time = SystemClock.uptimeMillis();
		}
		active = on;
	}
}
