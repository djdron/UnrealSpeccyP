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

package app.usp;

import java.nio.ByteBuffer;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class Audio
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
			bbuf.rewind();
			track.write(buf, 0, audio_bytes_ready);
			Emulator.the.ProfilerEnd(0);
		}
	}
	private AudioTrack track = null;
	private ByteBuffer bbuf = ByteBuffer.allocateDirect(65536);
	private byte[] buf = new byte[65536];
}
