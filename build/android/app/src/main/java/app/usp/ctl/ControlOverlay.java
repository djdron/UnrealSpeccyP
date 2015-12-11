/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2015 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

import android.os.SystemClock;

public class ControlOverlay
{
	protected boolean active = false;
	protected long touch_time = 0;

	static int NextPot(int v)
	{
		--v;
		v |= (v >> 1);
		v |= (v >> 2);
		v |= (v >> 4);
		v |= (v >> 8);
		v |= (v >> 16);
		return ++v;
	}
	public void Active(boolean on)
	{
		if(!active && on)
		{
			touch_time = SystemClock.uptimeMillis();
		}
		active = on;
	}
	public void KickVisible()
	{
		Active(false);
		Active(true);
	}
}
