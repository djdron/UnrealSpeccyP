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

import android.view.KeyEvent
import android.view.View
import app.usp.Emulator

class ControlKeys : View.OnKeyListener {

	private inner class KeyModifiers {
		var shift: Boolean = false
		var alt: Boolean = false
	}

	private fun TranslateKey(keyCode: Int, m: KeyModifiers): Int {
		return when (keyCode) {
			KeyEvent.KEYCODE_DPAD_LEFT -> 'l'.code
			KeyEvent.KEYCODE_DPAD_RIGHT -> 'r'.code
			KeyEvent.KEYCODE_DPAD_UP -> 'u'.code
			KeyEvent.KEYCODE_DPAD_DOWN -> 'd'.code
			KeyEvent.KEYCODE_DPAD_CENTER -> 'f'.code

			KeyEvent.KEYCODE_0 -> '0'.code
			KeyEvent.KEYCODE_1 -> '1'.code
			KeyEvent.KEYCODE_2 -> '2'.code
			KeyEvent.KEYCODE_3 -> '3'.code
			KeyEvent.KEYCODE_4 -> '4'.code
			KeyEvent.KEYCODE_5 -> '5'.code
			KeyEvent.KEYCODE_6 -> '6'.code
			KeyEvent.KEYCODE_7 -> '7'.code
			KeyEvent.KEYCODE_8 -> '8'.code
			KeyEvent.KEYCODE_9 -> '9'.code

			KeyEvent.KEYCODE_A -> 'A'.code
			KeyEvent.KEYCODE_B -> 'B'.code
			KeyEvent.KEYCODE_C -> 'C'.code
			KeyEvent.KEYCODE_D -> 'D'.code
			KeyEvent.KEYCODE_E -> 'E'.code
			KeyEvent.KEYCODE_F -> 'F'.code
			KeyEvent.KEYCODE_G -> 'G'.code
			KeyEvent.KEYCODE_H -> 'H'.code
			KeyEvent.KEYCODE_I -> 'I'.code
			KeyEvent.KEYCODE_J -> 'J'.code
			KeyEvent.KEYCODE_K -> 'K'.code
			KeyEvent.KEYCODE_L -> 'L'.code
			KeyEvent.KEYCODE_M -> 'M'.code
			KeyEvent.KEYCODE_N -> 'N'.code
			KeyEvent.KEYCODE_O -> 'O'.code
			KeyEvent.KEYCODE_P -> 'P'.code
			KeyEvent.KEYCODE_Q -> 'Q'.code
			KeyEvent.KEYCODE_R -> 'R'.code
			KeyEvent.KEYCODE_S -> 'S'.code
			KeyEvent.KEYCODE_T -> 'T'.code
			KeyEvent.KEYCODE_U -> 'U'.code
			KeyEvent.KEYCODE_V -> 'V'.code
			KeyEvent.KEYCODE_W -> 'W'.code
			KeyEvent.KEYCODE_X -> 'X'.code
			KeyEvent.KEYCODE_Y -> 'Y'.code
			KeyEvent.KEYCODE_Z -> 'Z'.code

			KeyEvent.KEYCODE_ENTER -> 'e'.code
			KeyEvent.KEYCODE_SPACE -> ' '.code
			KeyEvent.KEYCODE_SHIFT_LEFT,
			KeyEvent.KEYCODE_SHIFT_RIGHT -> 'c'.code
			KeyEvent.KEYCODE_ALT_LEFT,
			KeyEvent.KEYCODE_ALT_RIGHT -> 's'.code

//			KeyEvent.KEYCODE_MENU -> 'm'.code
//			KeyEvent.KEYCODE_BACK -> 'k'.code
			KeyEvent.KEYCODE_CALL -> 'e'.code
			KeyEvent.KEYCODE_CAMERA -> ' '.code
			KeyEvent.KEYCODE_DEL -> {
				m.shift = true
				'0'.code
			}
			KeyEvent.KEYCODE_COMMA -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'R'.code
				} else {
					'N'.code
				}
			}
			KeyEvent.KEYCODE_PERIOD -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'T'.code
				} else {
					'M'.code
				}
			}
			KeyEvent.KEYCODE_SLASH -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'C'.code
				} else {
					'V'.code
				}
			}
			KeyEvent.KEYCODE_TAB -> {
				m.alt = true
				m.shift = true
				0
			}
			KeyEvent.KEYCODE_SEMICOLON -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'Z'.code
				} else {
					'O'.code
				}
			}
			KeyEvent.KEYCODE_APOSTROPHE -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'P'.code
				} else {
					'7'.code
				}
			}
			KeyEvent.KEYCODE_EQUALS -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'K'.code
				} else {
					'L'.code
				}
			}
			KeyEvent.KEYCODE_MINUS -> {
				m.alt = true
				if (m.shift) {
					m.shift = false
					'0'.code
				} else {
					'J'.code
				}
			}
			KeyEvent.KEYCODE_BUTTON_A -> 'f'.code
			KeyEvent.KEYCODE_BUTTON_B -> 'u'.code
			KeyEvent.KEYCODE_BUTTON_X -> 'e'.code
//			KeyEvent.KEYCODE_BUTTON_Y -> ' '.code
			else -> -1
		}
	}

	override fun onKey(v: View, keyCode: Int, event: KeyEvent): Boolean {
		val m = KeyModifiers()
		m.shift = event.isShiftPressed
		m.alt = event.isAltPressed || event.isSymPressed

		val k = TranslateKey(keyCode, m)
		if (k < 0) return false

		if (event.action == KeyEvent.ACTION_DOWN) {
			Emulator.the.OnKey(k.toChar(), true, m.shift, m.alt)
		} else {
			Emulator.the.OnKey(k.toChar(), false, false, false)
		}
		return true
	}
}
