/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2011 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

package app.usp;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import android.content.Context;
import android.content.res.Configuration;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView;
import app.usp.ctl.ControlController;
import app.usp.ctl.ControlKeyboard;
import app.usp.ctl.ControlTouch;

public class ViewGLES extends GLSurfaceView
{
	public class Quad
	{
		Quad()
		{
			final float vertices[] =
			{
				-0.5f, -0.5f,
				+0.5f, -0.5f,
				+0.5f, +0.5f,
				-0.5f, +0.5f
			};
			v = ByteBuffer.allocateDirect(vertices.length*4).
					order(ByteOrder.nativeOrder()).asFloatBuffer();
			v.put(vertices).rewind();
			final float uv_coords[] =
			{
				0, 0,
				1, 0,
				1, 1,
				0, 1
			};
			uv = ByteBuffer.allocateDirect(uv_coords.length*4).
					order(ByteOrder.nativeOrder()).asFloatBuffer();
			uv.put(uv_coords).rewind();
			final byte triangles[] =
			{
				0, 1, 2,
				0, 2, 3,
			};
			t = ByteBuffer.allocateDirect(triangles.length);
			t.put(triangles).rewind();
		}
		public void Draw(GL10 gl)
		{
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, v);
			gl.glTexCoordPointer(2, GL10.GL_FLOAT, 0, uv);
		    gl.glDrawElements(GL10.GL_TRIANGLES, 2 * 3, GL10.GL_UNSIGNED_BYTE, t);
		}
		FloatBuffer v = null;
		FloatBuffer uv = null;
		ByteBuffer t = null;
	}
	private class SyncTimer
	{
		private long time_emulated = 0;
		private long time_real = 0;
		private long time_real_last = 0;
		public void Sync()
		{
			time_real += System.nanoTime() - time_real_last;
			final long TIME_FRAME = 20000000; // 20ms per frame - 50fps
			time_emulated += TIME_FRAME;
			final long TIME_DIFF = 10000000; // 10ms - sync diff
			while(time_emulated - time_real > TIME_DIFF)
			{
				final long t = System.nanoTime();
				try { Thread.sleep(1); } catch (InterruptedException e) {}
				time_real += System.nanoTime() - t;
			}
			if(time_emulated - time_real < 0)
			{
				time_real = 0;
				time_emulated = 0;
			}
			time_real_last = System.nanoTime();

//			while(System.nanoTime() - last_time < FRAME_TIME)
//			{
//				Thread.yield();
//				java.util.concurrent.locks.LockSupport.parkNanos(1);
//			}
//			last_time = System.nanoTime();
		}
	}
	private class Video extends ControlTouch implements Renderer
	{
		static final int WIDTH = 320;
		static final int HEIGHT = 240;
		static final int TEX_WIDTH = 512;
		static final int TEX_HEIGHT = 256;

		private ByteBuffer buf_video = ByteBuffer.allocateDirect(WIDTH*HEIGHT*2);
		private int[] textures = new int[1];
		private Quad quad = new Quad();
		private ControlController control_controller = null;
		private ControlKeyboard control_keyboard = null;
		private int width = 0;
		private int height = 0;
		boolean filtering = false;
		private float scale_x = 1.0f;
		private float scale_y = 1.0f;
		private SyncTimer sync_timer = null;
		Video(Context context)
		{
			control_controller = new ControlController(context);
			control_keyboard = new ControlKeyboard(context);
		}
		public void OnTouch(float x, float y, boolean down, int pid)
		{
			control_controller.OnTouch(x, height - y, down, pid);
			control_keyboard.OnTouch(x, y, down, pid);
		}
		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config)
		{
			Emulator.the.ProfilerBegin(2);
		    gl.glClearColor(0, 0, 0, 0);
			gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);
			gl.glEnableClientState(GL10.GL_TEXTURE_COORD_ARRAY);

			gl.glGenTextures(1, textures, 0);
			gl.glEnable(GL10.GL_TEXTURE_2D);

		    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
			gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0, GL10.GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL10.GL_RGB, GL10.GL_UNSIGNED_SHORT_5_6_5, null);

			control_controller.Init(gl);
			control_keyboard.Init(gl);

			gl.glMatrixMode(GL10.GL_PROJECTION);
			gl.glLoadIdentity();
			gl.glOrthof(-0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f);

			gl.glShadeModel(GL10.GL_FLAT);
			gl.glDisable(GL10.GL_DEPTH_TEST);
			gl.glDisable(GL10.GL_DITHER);
			gl.glDisable(GL10.GL_LIGHTING);
			gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_FASTEST);
		}
		@Override
		public void onDrawFrame(GL10 gl)
		{
			Emulator.the.ProfilerEnd(2);
			Emulator.the.Update();
			Emulator.the.UpdateVideo(buf_video);

			// write video buffer data to texture
			Emulator.the.ProfilerBegin(3);
		    gl.glBindTexture(GL10.GL_TEXTURE_2D, textures[0]);
			gl.glTexSubImage2D(GL10.GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL10.GL_RGB, GL10.GL_UNSIGNED_SHORT_5_6_5, buf_video);
			Emulator.the.ProfilerEnd(3);

			Emulator.the.ProfilerBegin(1);
			// draw emulator screen 
		    gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
			gl.glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    gl.glViewport(0, 0, width, height);
		    gl.glMatrixMode(GL10.GL_MODELVIEW);
		    gl.glLoadIdentity();
		    gl.glScalef(scale_x, scale_y, 1.0f);
			gl.glMatrixMode(GL10.GL_TEXTURE);
			gl.glLoadIdentity();
			gl.glScalef(((float)WIDTH)/TEX_WIDTH, ((float)HEIGHT)/TEX_HEIGHT, 1.0f);
			gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, filtering ? GL10.GL_LINEAR : GL10.GL_NEAREST);
			gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, filtering ? GL10.GL_LINEAR : GL10.GL_NEAREST);
			gl.glDisable(GL10.GL_BLEND);
			quad.Draw(gl);

			control_controller.Draw(gl, quad, width);
			control_keyboard.Draw(gl, quad);
			Emulator.the.ProfilerEnd(1);

			audio.Update();

			if(sync_timer != null)
				sync_timer.Sync();

			Emulator.the.ProfilerBegin(2);
		}
		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h)
		{
			width = w;
			height = h;
			filtering = Emulator.the.GetOptionBool(Preferences.filtering_id);
			if(Emulator.the.GetOptionBool(Preferences.av_timer_sync_id))
				sync_timer = new SyncTimer();
			else
				sync_timer = null;
		    final int zoom_mode = Emulator.the.GetOptionInt(Preferences.select_zoom_id);
			switch(zoom_mode)
			{
			case 0: // 1:1 mode
			    filtering = false;
			    scale_x = ((float)WIDTH) / w;
			    scale_y = ((float)HEIGHT) / h;
				break;
			default: // fill screen & others
				final float a = ((float)w)/h;
				final float a43 = ((float)4)/3;
				if(a > a43)
				{
					scale_x = a43/a;
					scale_y = 1.0f;
				}
				else
				{
					scale_x = 1.0f;
					scale_y = a/a43;
				}
				break;
			}
			float z = 1.0f;
			switch(zoom_mode)
			{
			case 2:	z = 300.0f/256.0f;	break; //small border
			case 3:	z = 320.0f/256.0f;	break; //no border
			}
			scale_x *= z;
			scale_y *= z;
		}
	}
	private Audio audio = null;
	private Video video = null;

	public ViewGLES(Context context)
	{
		super(context);
		setEGLConfigChooser(false);
		audio = new Audio();
		video = new Video(context);
		setRenderer(video);
		setOnTouchListener(video);
	}
	protected void onMeasure(int w, int h)
	{
		super.onMeasure(w, h);
		setFocusableInTouchMode(InLandscapeMode());
		OnControlsToggle();
	}
	protected boolean InLandscapeMode() { return getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE; }
	public void OnResume()	{ onResume(); }
	public void OnPause()	{ onPause(); }
	public void OnControlsToggle()
	{
		final boolean a = InLandscapeMode();
		final boolean k = Emulator.the.GetOptionBool(Preferences.use_keyboard_id);
		video.control_controller.Active(a && !k);
		video.control_keyboard.Active(a && k);
	}
}
