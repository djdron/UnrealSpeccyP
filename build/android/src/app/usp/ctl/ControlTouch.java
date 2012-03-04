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

package app.usp.ctl;

import android.view.MotionEvent;
import android.view.View;

public abstract class ControlTouch implements View.OnTouchListener
{
	abstract public void OnTouch(float x, float y, boolean down, int pid);
	public boolean onTouch(View view, MotionEvent event)
	{
		final int a = event.getAction() & MotionEvent.ACTION_MASK;
		if(a == MotionEvent.ACTION_MOVE)
		{
			for(int pidx = 0; pidx < event.getPointerCount(); ++pidx)
			{
				OnTouch(event.getX(pidx), event.getY(pidx), true, event.getPointerId(pidx));
			}
		}
		else
		{
			final boolean down = a == MotionEvent.ACTION_DOWN || a == MotionEvent.ACTION_POINTER_DOWN;
			final int pidx = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
			OnTouch(event.getX(pidx), event.getY(pidx), down, event.getPointerId(pidx));
		}
//		DumpEvent(event);
		return true;
	}
/*	private void DumpEvent(MotionEvent event)
	{
		final int a = event.getAction() & MotionEvent.ACTION_MASK;
		String action = "<unknown>";
		switch(a)
		{
		case MotionEvent.ACTION_DOWN:			action = "down";	break;
		case MotionEvent.ACTION_UP:				action = "up";		break;
		case MotionEvent.ACTION_POINTER_DOWN:	action = "p_down";	break;
		case MotionEvent.ACTION_POINTER_UP:		action = "p_up";	break;
		case MotionEvent.ACTION_MOVE:			action = "move";	break;
		case MotionEvent.ACTION_CANCEL:			action = "cancel";	break;
		}
		final int pidxe = (event.getAction() & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
		String dump = "pointer_" + event.getPointerId(pidxe) + action;
		for(int pidx = 0; pidx < event.getPointerCount(); ++pidx)
		{
			final float x = event.getX(pidx);
			final float y = event.getY(pidx);
			dump += ":(" + x + ", " + y + ")";
		}
		Log.d("touch", dump);
	}
*/
}
