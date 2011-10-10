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

import android.media.AudioTrack;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.content.Context;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView;

public class ViewGLES extends GLSurfaceView
{
	private class Video implements Renderer
	{
		static final int WIDTH = 320;
		static final int HEIGHT = 240;
		private ByteBuffer buf_video = ByteBuffer.allocateDirect(WIDTH*HEIGHT*2);
		int[] textures = new int[1];
		private final float vertices[] =
		{
			-0.5f, -0.5f,
			+0.5f, -0.5f,
			+0.5f, +0.5f,
			-0.5f, +0.5f
		};
		private final float uv_coords[] =
		{
			0, 0,
			1, 0,
			1, 1,
			0, 1
		};
		FloatBuffer v = null;
		FloatBuffer uv = null;
		private final byte triangles[] =
		{
			0, 1, 2,
			0, 2, 3,
		};
		ByteBuffer t = null;
		Video()
		{
			v = ByteBuffer.allocateDirect(vertices.length*4).
				order(ByteOrder.nativeOrder()).asFloatBuffer();
			v.put(vertices).rewind();
			uv = ByteBuffer.allocateDirect(uv_coords.length*4).
				order(ByteOrder.nativeOrder()).asFloatBuffer();
			uv.put(uv_coords).rewind();
			t = ByteBuffer.allocateDirect(triangles.length);
			t.put(triangles).rewind();
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

			final int TEX_WIDTH = 512;
			final int TEX_HEIGHT = 256;
			gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0, GL10.GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL10.GL_RGB, GL10.GL_UNSIGNED_SHORT_5_6_5, null);
			gl.glMatrixMode(GL10.GL_TEXTURE);
			gl.glLoadIdentity();
			gl.glScalef(((float)WIDTH)/TEX_WIDTH, ((float)HEIGHT)/TEX_HEIGHT, 1.0f);

			gl.glMatrixMode(GL10.GL_PROJECTION);
			gl.glLoadIdentity();
			gl.glOrthof(-0.5f, +0.5f, +0.5f, -0.5f, -1.0f, 1.0f);

			gl.glShadeModel(GL10.GL_FLAT);
			gl.glDisable(GL10.GL_DEPTH_TEST);
			gl.glDisable(GL10.GL_DITHER);
			gl.glDisable(GL10.GL_LIGHTING);
			gl.glDisable(GL10.GL_BLEND);
			gl.glHint(GL10.GL_PERSPECTIVE_CORRECTION_HINT, GL10.GL_FASTEST);
		}
		@Override
		public void onDrawFrame(GL10 gl)
		{
			Emulator.the.ProfilerEnd(2);
			Emulator.the.Update();
			Emulator.the.UpdateVideo(buf_video);

			Emulator.the.ProfilerBegin(3);
			gl.glTexSubImage2D(GL10.GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL10.GL_RGB, GL10.GL_UNSIGNED_SHORT_5_6_5, buf_video);
			Emulator.the.ProfilerEnd(3);
			Emulator.the.ProfilerBegin(1);
		    gl.glClear(GL10.GL_COLOR_BUFFER_BIT);
			gl.glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			gl.glVertexPointer(2, GL10.GL_FLOAT, 0, v);
			gl.glTexCoordPointer(2, GL10.GL_FLOAT, 0, uv);
		    gl.glDrawElements(GL10.GL_TRIANGLES, 2 * 3, GL10.GL_UNSIGNED_BYTE, t);
			Emulator.the.ProfilerEnd(1);

			audio.Update();
			Emulator.the.ProfilerBegin(2);
		}
		@Override
		public void onSurfaceChanged(GL10 gl, int w, int h)
		{
			boolean filtering;
		    float sx, sy;
		    final int zoom_mode = Emulator.the.GetOptionInt(Preferences.select_zoom_id);
			switch(zoom_mode)
			{
			case 0: // 1:1 mode
			    filtering = false;
			    sx = ((float)WIDTH) / w;
			    sy = ((float)HEIGHT) / h;
				break;
			default: // fill screen & others
			    filtering = true;
				final float a = ((float)w)/h;
				final float a43 = ((float)4)/3;
				if(a > a43)
				{
					sx = a43/a;
					sy = 1.0f;
				}
				else
				{
					sx = 1.0f;
					sy = a/a43;
				}
				break;
			}
			float z = 1.0f;
			switch(zoom_mode)
			{
			case 2:	z = 300.0f/256.0f;	break; //small border
			case 3:	z = 320.0f/256.0f;	break; //no border
			}
		    sx *= z;
		    sy *= z;

		    gl.glViewport(0, 0, w, h);
		    gl.glMatrixMode(GL10.GL_MODELVIEW);
		    gl.glLoadIdentity();
		    gl.glScalef(sx, sy, 1.0f);

			gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MIN_FILTER, filtering ? GL10.GL_LINEAR : GL10.GL_NEAREST);
			gl.glTexParameterf(GL10.GL_TEXTURE_2D, GL10.GL_TEXTURE_MAG_FILTER, filtering ? GL10.GL_LINEAR : GL10.GL_NEAREST);
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

	public ViewGLES(Context context)
	{
		super(context);
		setEGLConfigChooser(false);
		audio = new Audio();
		video = new Video();
		setRenderer(video);
	}
	public void OnResume()	{ onResume(); }
	public void OnPause()	{ onPause(); }
}
