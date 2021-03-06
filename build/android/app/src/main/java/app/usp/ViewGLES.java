/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2020 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

import android.content.Context;
import android.content.res.Configuration;
import android.opengl.GLSurfaceView;
import android.os.Handler;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.view.KeyEvent;
import android.widget.Toast;

import app.usp.ctl.ControlKeys;
import app.usp.ctl.ControlOverlayController;
import app.usp.ctl.ControlOverlayFastForward;
import app.usp.ctl.ControlOverlayKeyboard;
import app.usp.ctl.ControlOverlayReplay;
import app.usp.ctl.ControlSensor;
import app.usp.ctl.ControlTouch;
import app.usp.ctl.Control;

public class ViewGLES extends GLSurfaceView
{
	private class SyncTimer
	{
		private long time_emulated = 0;
		private long time_real = 0;
		private long time_real_last = 0;
		static final long TIME_FRAME = 20000000; // 20ms per frame - 50fps
		public void Sync()
		{
			time_real += System.nanoTime() - time_real_last;
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

		private int[] textures = new int[1];
		private ControlOverlayController control_controller;
		private ControlOverlayKeyboard control_keyboard;
		private ControlOverlayReplay control_replay;
		private ControlOverlayFastForward control_fast_forward;

		private int width = 0;
		private int height = 0;
		private SyncTimer sync_timer = null;
		private Context context;
		Video(Context _context)
		{
			context = _context;
			control_controller = new ControlOverlayController(context);
			control_keyboard = new ControlOverlayKeyboard(context);
			control_replay = new ControlOverlayReplay(context);
			control_fast_forward = new ControlOverlayFastForward(context);
		}
		public void OnTouch(float x, float y, boolean down, int pid)
		{
			control_replay.OnTouch(x, y, down, pid);
			if(control_fast_forward.OnTouch(width - x, y, down, pid))
				return;
			control_controller.OnTouch(x, height - y, down, pid);
			control_keyboard.OnTouch(x, y, down, pid);
		}
		@Override
		public void onSurfaceCreated(GL10 gl_unused, EGLConfig config)
		{
			Emulator.the.GLDone();
			Emulator.the.GLInit();
			Emulator.the.ProfilerBegin(2);

			control_controller.Init();
			control_keyboard.Init();
			control_replay.Init();
			control_fast_forward.Init();
		}
		private void ShowMessage(final int code)
		{
			if(code > 0)
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
		private void Draw()
		{
			Emulator.the.ProfilerBegin(3);
			Emulator.the.GLDraw(width, height);
			Emulator.the.ProfilerEnd(3);

			Emulator.the.ProfilerBegin(1);
			control_controller.Draw(width);
			control_keyboard.Draw(width, height);
			control_replay.Draw(width, height);
			control_fast_forward.Draw(width, height);
			Emulator.the.ProfilerEnd(1);
		}
		@Override
		public void onDrawFrame(GL10 gl_unused)
		{
			Emulator.the.ProfilerEnd(2);
			if(control_fast_forward.Pressed())
			{
				final long time_frame_end = System.nanoTime() + SyncTimer.TIME_FRAME*4/5;
				ShowMessage(Emulator.the.Update());
				Draw();
				audio.Update(false);
				for(int frames = 1; frames < 15; ++frames) // do not speedup faster than 15x
				{
					if(System.nanoTime() > time_frame_end)
						break;
					ShowMessage(Emulator.the.Update());
					audio.Update(true);
				}
			}
			else
			{
				final int skip_frames = Emulator.the.GetOptionInt(Preferences.skip_frames_id);
				for(int f = 0; f <= skip_frames; ++f)
				{
					ShowMessage(Emulator.the.Update());
					if(f == skip_frames)
					{
						Draw();
					}
					audio.Update(false);
					if(sync_timer != null)
						sync_timer.Sync();
				}
			}
			Emulator.the.ProfilerBegin(2);
		}
		@Override
		public void onSurfaceChanged(GL10 gl_unused, int w, int h)
		{
			width = w;
			height = h;
			if(Emulator.the.GetOptionBool(Preferences.av_timer_sync_id))
				sync_timer = new SyncTimer();
			else
				sync_timer = null;
		}
	}
	private Audio audio;
	private Video video;
	private Control control;
	private ControlSensor sensor;
	private Main main_activity;

	public ViewGLES(Main _main_activity, Control _control)
	{
		super(_main_activity);
		setEGLContextClientVersion(2);
		setEGLConfigChooser(false);
		control = _control;
		main_activity = _main_activity;
		audio = new Audio();
		video = new Video(main_activity);
		sensor = new ControlSensor(main_activity);
		setFocusable(true);
		setFocusableInTouchMode(true);
		setOnKeyListener(new ControlKeys());
		setRenderer(video);
		setOnTouchListener(video);
	}
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		switch(keyCode)
		{
		case KeyEvent.KEYCODE_BACK:
			if(!event.isLongPress())
			{
				if(main_activity.IsTV())
				{
					main_activity.OpenMenu();
				}
				else
				{
					control.OnToggle();
					UpdateControls();
				}
				return true;
			}
			break;
		case KeyEvent.KEYCODE_F1:
		case KeyEvent.KEYCODE_BUTTON_START:
			main_activity.OpenMenu();
			return true;
		}
		return super.onKeyUp(keyCode, event);
	}
	@Override
	protected void onMeasure(int w, int h)
	{
		super.onMeasure(w, h);
		UpdateControls();
	}
	private boolean InLandscapeMode() { return getResources().getConfiguration().orientation == Configuration.ORIENTATION_LANDSCAPE; }
	public void OnActivityResume()	{ onResume();	sensor.Install(); }
	public void OnActivityPause()	{ onPause();	sensor.Uninstall(); }
	private void UpdateControls()
	{
		final boolean a = InLandscapeMode();
		final boolean k = Emulator.the.GetOptionBool(Preferences.use_keyboard_id);
		video.control_controller.Active(a && !k);
		video.control_keyboard.Active(a && k);
		video.control_replay.KickVisible();
		video.control_fast_forward.KickVisible();
	}
}
