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
import android.opengl.GLUtils;
import android.os.SystemClock;

import app.usp.Emulator;
import app.usp.ViewGLES;

public class ControlReplay extends ControlOverlay
{
	private final int size;
	private final float scale_pot;
	private int[] textures = new int[2];
	private Bitmap pos_fill = null;
	private Bitmap pos_dot = null;
	static final int HIDE_TIME_MS = 4000;

	public ControlReplay(Context context)
	{
		size = (int)(context.getResources().getDisplayMetrics().density*9);
		final int size_pot = NextPot(size);
		scale_pot = ((float)size)/size_pot;

		Paint paint = new Paint();
		paint.setAntiAlias(true);

		pos_fill = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888);
		Canvas canvas = new Canvas(pos_fill);
		paint.setColor(Color.WHITE);
		canvas.drawRect(0, size/4, size, size*3/4, paint);

		pos_dot = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888);
		canvas = new Canvas(pos_dot);
		paint.setColor(Color.WHITE);
		canvas.drawCircle(size*0.5f, size*0.5f, size*0.5f, paint);
	}
	public void Init(GL10 gl)
	{
		gl.glGenTextures(2, textures, 0);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
		GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, pos_fill, 0);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[1]);
		GLUtils.texImage2D(GL10.GL_TEXTURE_2D, 0, pos_dot, 0);
	}
	public void OnTouch(float x, float y, boolean down, int pointer_id)
	{
		if(!active)
			return;
		touch_time = SystemClock.uptimeMillis();
	}
	public void Draw(GL10 gl, ViewGLES.Quad quad, int _w, int _h)
	{
		if(!active)
			return;
		final long passed_time = SystemClock.uptimeMillis() - touch_time;
		if(passed_time > HIDE_TIME_MS)
			return;

		Emulator.ReplayProgress progress = Emulator.the.ReplayProgress();
		if(progress == null)
			return;

		final float alpha = passed_time > (HIDE_TIME_MS - 1000) ? (float)(HIDE_TIME_MS - passed_time)/1000.0f : 1.0f;

		int px = (int)(_w*0.15f);
		int py = (int)(_h*0.05f);

		float p = (float)progress.frame_current/progress.frames_total;
		int width = _w - px*2;
		int psx = (int)(width*p);

		// draw progress
		gl.glColor4f(0.5f, 0.0f, 0.0f, 0.8f * alpha);
		gl.glViewport(px, py, psx, size);
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

		int psxc = psx;
		if(progress.frame_current + progress.frames_cached < progress.frames_total)
		{
			float pc = (float)(progress.frame_current + progress.frames_cached)/progress.frames_total;
			psxc = (int)(width*pc);

			gl.glColor4f(0.4f, 0.4f, 0.4f, 0.8f*alpha);
			gl.glViewport(px + psx, py, psxc - psx, size);
			quad.Draw(gl);
		}

		gl.glColor4f(0.2f, 0.2f, 0.2f, 0.8f*alpha);
		gl.glViewport(px + psxc, py, width - psxc, size);
		quad.Draw(gl);

		gl.glColor4f(1.0f, 1.0f, 1.0f, 1.0f*alpha);
		gl.glViewport(px + psx - size/2, py, size, size);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[1]);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_NEAREST);
		gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_NEAREST);

		quad.Draw(gl);
	}
}
