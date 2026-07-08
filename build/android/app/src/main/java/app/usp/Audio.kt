/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2026 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

package app.usp

import java.nio.ByteBuffer

import android.media.AudioFormat
import android.media.AudioAttributes
import android.media.AudioTrack

class Audio {

	private val bbuf: ByteBuffer = ByteBuffer.allocateDirect(65536)
	private val buf: ByteArray = ByteArray(65536)

	private val track: AudioTrack = run {
		val freq = 44100
		val channels = AudioFormat.CHANNEL_OUT_STEREO
		val format = AudioFormat.ENCODING_PCM_16BIT
		val buf_size = AudioTrack.getMinBufferSize(freq, channels, format)

		AudioTrack.Builder()
			.setAudioAttributes(
				AudioAttributes.Builder()
					.setUsage(AudioAttributes.USAGE_GAME)
					.setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
					.build()
			)
			.setAudioFormat(
				AudioFormat.Builder()
					.setSampleRate(freq)
					.setChannelMask(channels)
					.setEncoding(format)
					.build()
			)
			.setBufferSizeInBytes(buf_size + 44100*2*2*5/50) // 5-frame additional size
			.setTransferMode(AudioTrack.MODE_STREAM)
			.build()
	}

	init {
		track.play()
	}

	fun Update(skip_data: Boolean) {
		val audio_bytes_ready = Emulator.the.UpdateAudio(bbuf, skip_data)
		if (audio_bytes_ready != 0) {
			Emulator.the.ProfilerBegin(0)
			bbuf.rewind()
			bbuf.get(buf)
			bbuf.rewind()
			track.write(buf, 0, audio_bytes_ready)
			Emulator.the.ProfilerEnd(0)
		}
	}
}
