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
import android.graphics.Canvas;
import android.graphics.Bitmap;
import android.graphics.Paint;
import android.media.AudioTrack;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.content.Context;

public class View extends SurfaceView  implements Callback
{
	private class Video
	{
		static final int WIDTH = 320;
		static final int HEIGHT = 240;
		private Bitmap bmp = Bitmap.createBitmap(WIDTH, HEIGHT, Bitmap.Config.RGB_565);
		private ByteBuffer buf_video = ByteBuffer.allocateDirect(WIDTH*HEIGHT*2);
		private float scale = 1.0f;
		private Paint paint_flags = null;
		private int frame = 0;
		private int skip_frames = 1;
		final boolean Update()
		{
			++frame;
			Emulator.the.Update();
			final boolean update_video = (frame % skip_frames) == 0;
			if(update_video)
			{
				Emulator.the.UpdateVideo(buf_video);
				Emulator.the.ProfilerBegin(1);
				buf_video.rewind();
				bmp.copyPixelsFromBuffer(buf_video);
				Emulator.the.ProfilerEnd(1);
			}
			return update_video;
		}
	}
	private class Audio
	{
		Audio()
		{
			final int freq = 44100;
			final int channels = AudioFormat.CHANNEL_CONFIGURATION_STEREO;
			final int format = AudioFormat.ENCODING_PCM_16BIT;
			final int buf_size = AudioTrack.getMinBufferSize(freq, channels, format);
			track = new AudioTrack(	AudioManager.STREAM_MUSIC,
					freq, channels, format, buf_size*4,
					AudioTrack.MODE_STREAM);
			track.play();
		}
		void Update()
		{
			final int audio_bytes_ready = Emulator.the.UpdateAudio(bbuf);
			if(audio_bytes_ready != 0)
			{
				Emulator.the.ProfilerBegin(0);
				bbuf.rewind();
				bbuf.get(buf);
				track.write(buf, 0, audio_bytes_ready);
				Emulator.the.ProfilerEnd(0);
			}
		}
		private AudioTrack track;
		private ByteBuffer bbuf = ByteBuffer.allocateDirect(32768);
		private byte[] buf = new byte[32768];
	}
	private Audio audio = null;
	private Video video = null;
	private Thread thread = null;
	private boolean thread_exit = false;
	private static class Lock {};
	private final Object lock_scr = new Lock();
	private SurfaceHolder sh = null;

	public View(Context context)
	{
		super(context);
		getHolder().addCallback(this);
		audio = new Audio();
		video = new Video();
	}
	private void StartRenderThread()
	{
		thread_exit = false;
		if(thread != null)
			return;
		thread = new Thread(new Runnable()
		{
			public void run()
			{
				while(!thread_exit)
				{
					if(sh != null)
						Draw();
					else
						Thread.yield();
				}
			}
		},"draw");
		thread.start();
	}
	private void StopRenderThread()
	{
		if(thread != null)
		{
			thread_exit = true;
			try{ thread.join(); } catch(InterruptedException e) {}
			thread = null;
		}
	}
	public void surfaceCreated(SurfaceHolder holder)
	{
		synchronized(lock_scr) { sh = holder; }
	}
	public void surfaceDestroyed(SurfaceHolder holder)
	{
		synchronized(lock_scr) { sh = null; }
	}
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height)
	{
		synchronized(lock_scr) { sh = holder; }
	}
	@Override
	protected void onMeasure(int _w, int _h)
	{
		int w = (int)(Video.WIDTH*video.scale);
		int h = (int)(Video.HEIGHT*video.scale);
		setMeasuredDimension(w, h);
	}
	private void Draw()
	{
		final boolean update_video = video.Update();
		audio.Update();
		if(update_video) synchronized(lock_scr)
		{
			if(sh != null)
			{
				Emulator.the.ProfilerBegin(2);
				Canvas c = sh.lockCanvas();
				c.save();
				c.scale(video.scale, video.scale);
				c.drawBitmap(video.bmp, 0, 0, video.paint_flags);
				c.restore();
				sh.unlockCanvasAndPost(c);
				Emulator.the.ProfilerEnd(2);
			}
		}
	}
	public void OnResume()	{ StartRenderThread(); }
	public void OnPause()	{ StopRenderThread(); }
	public void SetSkipFrames(int sf)
	{
		if(sf != 0)
		{
			video.skip_frames = (1 << sf) + 1;
		}
		else
			video.skip_frames = 1;
	}
	public void SetZoom(int zoom, int w, int h)
	{
		if(zoom != 0)
		{
			final float xs = ((float)w)/Video.WIDTH;
			final float ys = ((float)h)/Video.HEIGHT;
			video.scale = xs < ys ? xs : ys;
		}
		else
			video.scale = 1.0f;
	}
	public void SetFiltering(boolean on)
	{
		if(on)
		{
			if(video.paint_flags == null)
				video.paint_flags = new Paint(Paint.FILTER_BITMAP_FLAG);
		}
		else
			video.paint_flags = null;
	}
}
