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

public class Emulator
{
	static
	{
		System.loadLibrary("usp");
	}
	synchronized native void	InitResources(final ByteBuffer rom0, final ByteBuffer rom1, final ByteBuffer rom2, final ByteBuffer rom3);
	synchronized native void	Init(final String path);
	synchronized native void	Done();

	synchronized native void	Update();
	synchronized native void	UpdateVideo(ByteBuffer buf);
	synchronized native int		UpdateAudio(ByteBuffer buf);
	synchronized native void	OnKey(char key, boolean down, boolean shift, boolean alt);
	synchronized native void	OnTouch(boolean keyboard, float x, float y, boolean down, int pointer_id);

	synchronized public native boolean	Open(final String file);
	synchronized public native String	GetLastFile();
	synchronized native void	LoadState();
	synchronized native void	SaveState();
	synchronized native void	Reset();
	synchronized public native boolean	FileTypeSupported(final String name);

	synchronized native int		GetOptionInt(final String name);
	synchronized native void	SetOptionInt(final String name, int value);
	synchronized native boolean	GetOptionBool(final String name);
	synchronized native void	SetOptionBool(final String name, boolean value);
	synchronized native void	StoreOptions();

	synchronized native int		TapeState();
	synchronized native void	TapeToggle();

	public static Emulator the = new Emulator();
}
