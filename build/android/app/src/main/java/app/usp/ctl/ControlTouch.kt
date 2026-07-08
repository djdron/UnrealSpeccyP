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

package app.usp.ctl

import android.util.Log
import android.view.MotionEvent
import android.view.View

abstract class ControlTouch : View.OnTouchListener {

	abstract fun OnTouch(x: Float, y: Float, down: Boolean, pid: Int)

	override fun onTouch(view: View, event: MotionEvent): Boolean {
		val a = event.action and MotionEvent.ACTION_MASK

		if (a == MotionEvent.ACTION_MOVE) {
			for (pidx in 0 until event.pointerCount) {
				OnTouch(event.getX(pidx), event.getY(pidx), true, event.getPointerId(pidx))
			}
		} else {
			val down = a == MotionEvent.ACTION_DOWN || a == MotionEvent.ACTION_POINTER_DOWN
			val pidx = (event.action and MotionEvent.ACTION_POINTER_INDEX_MASK) shr MotionEvent.ACTION_POINTER_INDEX_SHIFT
			OnTouch(event.getX(pidx), event.getY(pidx), down, event.getPointerId(pidx))
		}
		// DumpEvent(event)
		return true
	}

/*	private fun DumpEvent(event: MotionEvent) {
		val a = event.action and MotionEvent.ACTION_MASK
		val action = when (a) {
			MotionEvent.ACTION_DOWN -> "down"
			MotionEvent.ACTION_UP -> "up"
			MotionEvent.ACTION_POINTER_DOWN -> "p_down"
			MotionEvent.ACTION_POINTER_UP -> "p_up"
			MotionEvent.ACTION_MOVE -> "move"
			MotionEvent.ACTION_CANCEL -> "cancel"
			else -> "<unknown>"
		}

		val pidxe = (event.action and MotionEvent.ACTION_POINTER_INDEX_MASK) shr MotionEvent.ACTION_POINTER_INDEX_SHIFT
		var dump = "pointer_" + event.getPointerId(pidxe) + action

		for (pidx in 0 until event.pointerCount) {
			val x = event.getX(pidx)
			val y = event.getY(pidx)
			dump += ":($x, $y)"
		}
		Log.d("touch", dump)
	}
*/
}
