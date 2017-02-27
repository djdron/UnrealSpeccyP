/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2017 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.os.SystemClock;
import app.usp.Emulator;
import app.usp.R;

public class ControlKeyboard extends ControlOverlay
{
	private int width;
	private int height;
	private int pid_activator = -1;
	static final int HIDE_TIME_MS = 4000;
	private int	sprite = -1;
	private int sprite_width;
	private int sprite_height;

	private int[] textures = new int[1];
	private Bitmap keyboard = null;

	public ControlKeyboard(Context context)
	{
		//@note : width & height may vary at runtime a little ;-) 
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
		Bitmap bm = null;
		if(width > 480)
			bm = BitmapFactory.decodeResource(context.getResources(), R.drawable.keyboard_hq, options);
		else
			bm = BitmapFactory.decodeResource(context.getResources(), R.drawable.keyboard, options);
		bm.setDensity(Bitmap.DENSITY_NONE);
		sprite_width = bm.getWidth();
		sprite_height = bm.getHeight();
		final int width_pot = NextPot(sprite_width);
		final int height_pot = NextPot(sprite_height);

        keyboard = Bitmap.createBitmap(width_pot, height_pot, Bitmap.Config.RGB_565);
	    Paint paint = new Paint();
	    paint.setAntiAlias(true);
		Canvas canvas = new Canvas(keyboard);
		canvas.drawBitmap(bm, 0.0f, 0.0f, paint);
	}
	public void Init()
	{
		GLES20.glGenTextures(1, textures, 0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
	    GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, keyboard, 0);

		sprite = Emulator.the.GLSpriteCreate(sprite_width, sprite_height);
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
	public void Draw(int _w, int _h)
	{
		if(!active)
			return;
		if(Emulator.the.ReplayActive())
			return;
		width = _w;
		height = _h;
		final long passed_time = SystemClock.uptimeMillis() - touch_time;
		if(passed_time > HIDE_TIME_MS)
		{
			return;
		}

		final float alpha = passed_time > (HIDE_TIME_MS - 1000) ? (float)(HIDE_TIME_MS - passed_time)/1000.0f : 1.0f;
		// draw keyboard
		Emulator.the.GLSpriteDraw(sprite, textures[0], 0, 0, width, height, 0.5f*alpha, true);
	}
}
