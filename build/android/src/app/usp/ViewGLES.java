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
//import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.os.Handler;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.widget.Toast;
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
		public void Draw()
		{
//			GLES20.glVertexPointer(2, GLES20.GL_FLOAT, 0, v);
//			GLES20.glTexCoordPointer(2, GLES20.GL_FLOAT, 0, uv);
//		    GLES20.glDrawElements(GLES20.GL_TRIANGLES, 2 * 3, GLES20.GL_UNSIGNED_BYTE, t);
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
		private Context context;
		Video(Context _context)
		{
			context = _context;
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
			Emulator.the.InitGL();
			Emulator.the.ProfilerBegin(2);
/*		    GLES20.glClearColor(0, 0, 0, 0);
			GLES20.glEnableClientState(GLES20.GL_VERTEX_ARRAY);
			GLES20.glEnableClientState(GLES20.GL_TEXTURE_COORD_ARRAY);

			GLES20.glGenTextures(1, textures, 0);
			GLES20.glEnable(GLES20.GL_TEXTURE_2D);

		    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
			GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GLES20.GL_RGB, GLES20.GL_UNSIGNED_SHORT_5_6_5, null);

			control_controller.Init(gl);
			control_keyboard.Init(gl);

			GLES20.glMatrixMode(GLES20.GL_PROJECTION);
			GLES20.glLoadIdentity();
			GLES20.glOrthof(-0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f);

			GLES20.glShadeModel(GLES20.GL_FLAT);
			GLES20.glDisable(GLES20.GL_DEPTH_TEST);
			GLES20.glDisable(GLES20.GL_DITHER);
			GLES20.glDisable(GLES20.GL_LIGHTING);
			GLES20.glHint(GLES20.GL_PERSPECTIVE_CORRECTION_HINT, GLES20.GL_FASTEST);
*/
		}
		private void ShowMessage(final int code)
		{
			if(code != 0)
			{
				Handler h = new Handler(context.getMainLooper());
				h.post(new Runnable()
				{
					@Override
					public void run()
					{
						String msg = null;
						switch(code)
						{
						case 1:	msg = context.getString(R.string.rzx_finished);		break;
						case 2:	msg = context.getString(R.string.rzx_sync_lost);	break;
						case 3:	msg = context.getString(R.string.rzx_invalid);		break;
						case 4:	msg = context.getString(R.string.rzx_unsupported);	break;
						}
						if(msg != null)
						{
							Toast.makeText(context.getApplicationContext(), msg, Toast.LENGTH_LONG).show();
						}
					}
				});
			}
		}
		@Override
		public void onDrawFrame(GL10 _gl)
		{
			Emulator.the.ProfilerEnd(2);
			ShowMessage(Emulator.the.Update());
			Emulator.the.DrawGL(width, height);

/*			// write video buffer data to texture
			Emulator.the.ProfilerBegin(3);
		    GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
			GLES20.glTexSubImage2D(GLES20.GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GLES20.GL_RGB, GLES20.GL_UNSIGNED_SHORT_5_6_5, buf_video);
			Emulator.the.ProfilerEnd(3);

			Emulator.the.ProfilerBegin(1);
			// draw emulator screen 
		    GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);
			GLES20.glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		    GLES20.glViewport(0, 0, width, height);
		    GLES20.glMatrixMode(GLES20.GL_MODELVIEW);
		    GLES20.glLoadIdentity();
		    GLES20.glScalef(scale_x, scale_y, 1.0f);
			GLES20.glMatrixMode(GLES20.GL_TEXTURE);
			GLES20.glLoadIdentity();
			GLES20.glScalef(((float)WIDTH)/TEX_WIDTH, ((float)HEIGHT)/TEX_HEIGHT, 1.0f);
			GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER, filtering ? GLES20.GL_LINEAR : GLES20.GL_NEAREST);
			GLES20.glTexParameterf(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER, filtering ? GLES20.GL_LINEAR : GLES20.GL_NEAREST);
			GLES20.glDisable(GLES20.GL_BLEND);
			quad.Draw(gl);

			control_controller.Draw(gl, quad, width);
			control_keyboard.Draw(gl, quad);
			Emulator.the.ProfilerEnd(1);
*/

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
		setEGLContextClientVersion(2);
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
