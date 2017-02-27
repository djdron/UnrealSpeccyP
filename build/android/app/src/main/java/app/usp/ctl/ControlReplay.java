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

import android.graphics.Bitmap;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.os.SystemClock;
import android.text.TextUtils;

import app.usp.Emulator;

public class ControlReplay extends ControlOverlay
{
	private final int size;
	private int[] textures = new int[2];
	private Bitmap pos_fill = null;
	private Bitmap pos_dot = null;
	static final int HIDE_TIME_MS = 4000;

	private TimeLabel time_current = null;
	private TimeLabel time_total = null;

	private int sprite = -1;

	class TextLabel
	{
		Paint paint = null;
		private Bitmap bmp = null;
		private int[] textures = new int[1];
		public int width, height;
		protected String text;
		private int sprite = -1;

		TextLabel(Context context, int text_size_dp, String max_text)
		{
			text = "";
			paint = new Paint();
			paint.setAntiAlias(true);
			// 10 dp font size
			int textSize = (int)(text_size_dp * context.getResources().getDisplayMetrics().density + 0.5f);
			paint.setTextSize(textSize);
			paint.setTextAlign(Paint.Align.CENTER);
			width = (int)paint.measureText(max_text) + 5;
			height = textSize + 5;
			final int width_pot = NextPot(width);
			final int height_pot = NextPot(height);
			bmp = Bitmap.createBitmap(width_pot, height_pot, Bitmap.Config.ARGB_8888);
		}
		public void Init()
		{
			GLES20.glGenTextures(1, textures, 0);
			GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
			GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0);
			sprite = Emulator.the.GLSpriteCreate(width, height);
		}
		void Draw(String _text, int x, int y, float alpha)
		{
			if(TextUtils.isEmpty(_text))
				return;

			if(text.compareTo(_text) != 0)
			{
				text = _text;
				Canvas canvas = new Canvas(bmp);
				canvas.clipRect(0, 0, width, height);
				canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR);
				paint.setColor(Color.BLACK);
				canvas.drawText(text, width/2 + 3, height - 4, paint);
				paint.setColor(Color.WHITE);
				canvas.drawText(text, width/2 + 2, height - 5, paint);
				GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
				GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bmp, 0);
			}
			Emulator.the.GLSpriteDraw(sprite, textures[0], x, y, width, height, alpha*0.998f, false); // to get text always with alpha blend
		}
	}

	class TimeLabel  extends TextLabel
	{
		private int seconds = -1;
		TimeLabel(Context context, int text_size_dp)
		{
			super(context, text_size_dp, "88:88:88");
		}
		void Draw(int _seconds, int x, int y, float alpha)
		{
			if(seconds == _seconds)
			{
				super.Draw(super.text, x, y, alpha);
				return;
			}
			seconds = _seconds;
			int h = seconds / 3600;
			int m = (seconds % 3600) / 60;
			int s = seconds % 60;
			String time;
			if(h > 0)
				time = String.format("%02d:%02d:%02d", h, m, s);
			else
				time = String.format("%02d:%02d", m, s);
			super.Draw(time, x, y, alpha);
		}
	}

	public ControlReplay(Context context)
	{
		size = (int)(context.getResources().getDisplayMetrics().density*9);
		final int size_pot = NextPot(size);

		Paint paint = new Paint();
		paint.setAntiAlias(true);

		pos_fill = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888);
		Canvas canvas = new Canvas(pos_fill);
		paint.setColor(Color.WHITE);
		canvas.drawRect(0, size*2/9, size, size*7/9, paint);

		pos_dot = Bitmap.createBitmap(size_pot, size_pot, Bitmap.Config.ARGB_8888);
		canvas = new Canvas(pos_dot);
		paint.setColor(Color.WHITE);
		canvas.drawCircle(size*0.5f, size*0.5f, size*0.5f, paint);

		time_current = new TimeLabel(context, 12);
		time_total = new TimeLabel(context, 12);
	}
	public void Init()
	{
		GLES20.glGenTextures(2, textures, 0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, pos_fill, 0);
		GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[1]);
		GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, pos_dot, 0);
		time_current.Init();
		time_total.Init();

		sprite = Emulator.the.GLSpriteCreate(size, size);
	}
	public void OnTouch(float x, float y, boolean down, int pointer_id)
	{
		if(!active)
			return;
		touch_time = SystemClock.uptimeMillis();
	}
	public void Draw(int _w, int _h)
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

		int px = (int)(time_current.width*1.2f);
		int py = (int)(_h*0.05f);

		float p = (float)progress.frame_current/progress.frames_total;
		int width = _w - px*2;
		int psx = (int)(width*p);

		// draw progress
		Emulator.the.GLSpriteSetColor(sprite, 0.5f, 0.0f, 0.0f);
		Emulator.the.GLSpriteDraw(sprite, textures[0], px, py, psx, size, 0.8f * alpha, false);

		int psxc = psx;
		if(progress.frame_current + progress.frames_cached < progress.frames_total)
		{
			float pc = (float)(progress.frame_current + progress.frames_cached)/progress.frames_total;
			psxc = (int)(width*pc);
			Emulator.the.GLSpriteSetColor(sprite, 0.4f, 0.4f, 0.4f);
			Emulator.the.GLSpriteDraw(sprite, textures[0], px + psx, py, psxc - psx, size, 0.8f * alpha, false);
		}

		Emulator.the.GLSpriteSetColor(sprite, 0.2f, 0.2f, 0.2f);
		Emulator.the.GLSpriteDraw(sprite, textures[0], px + psxc, py, width - psxc, size, 0.8f * alpha, false);

		Emulator.the.GLSpriteSetColor(sprite, 1.0f, 1.0f, 1.0f);
		Emulator.the.GLSpriteDraw(sprite, textures[1], px + psx - size/2, py, size, size, 0.8f * alpha, false);

		time_current.Draw(progress.frame_current/50, (int)(time_current.width*0.05f), py + size/2 - time_current.height/2, 1.0f*alpha);
		time_total.Draw(progress.frames_total/50, _w - (int)(time_current.width*1.05f), py + size/2 - time_current.height/2, 1.0f*alpha);
	}
}
