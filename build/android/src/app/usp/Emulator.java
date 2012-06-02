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
	synchronized native void	InitRom(final int rom_id, final ByteBuffer rom);
	synchronized native void	InitFont(final ByteBuffer fnt);
	synchronized native void	Init(final String path);
	synchronized native void	Done();

	synchronized native void	Update();
	synchronized native void	UpdateVideo(ByteBuffer buf);
	synchronized native int	UpdateAudio(ByteBuffer buf);
	public synchronized native void	OnKey(char key, boolean down, boolean shift, boolean alt);
	public synchronized native void	OnTouch(boolean keyboard, float x, float y, boolean down, int pointer_id);

	public synchronized native boolean	Open(final String file);
	public synchronized native String	GetLastFile();
	synchronized native void	LoadState();
	synchronized native void	SaveState();
	synchronized native void	Reset();
	public synchronized native boolean	FileTypeSupported(final String name);

	synchronized native int	GetOptionInt(final String name);
	synchronized native void	SetOptionInt(final String name, int value);
	public synchronized native boolean GetOptionBool(final String name);
	public synchronized native void	SetOptionBool(final String name, boolean value);
	synchronized native void	StoreOptions();

	synchronized native int	TapeState();
	synchronized native void	TapeToggle();
	
	synchronized native void	ProfilerBegin(int id);
	synchronized native void	ProfilerEnd(int id);

	public static Emulator the = new Emulator();
}
