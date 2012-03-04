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

import android.view.KeyEvent;
import android.view.View;
import app.usp.Emulator;

public class ControlKeyboard implements View.OnKeyListener
{
	private final char TranslateKey(int keyCode)
	{
		switch(keyCode)
		{
		case KeyEvent.KEYCODE_DPAD_LEFT:		return 'l';
		case KeyEvent.KEYCODE_DPAD_RIGHT:		return 'r';
		case KeyEvent.KEYCODE_DPAD_UP:			return 'u';
		case KeyEvent.KEYCODE_DPAD_DOWN:		return 'd';
		case KeyEvent.KEYCODE_DPAD_CENTER:		return 'f';

		case KeyEvent.KEYCODE_0:				return '0';
		case KeyEvent.KEYCODE_1:				return '1';
		case KeyEvent.KEYCODE_2:				return '2';
		case KeyEvent.KEYCODE_3:				return '3';
		case KeyEvent.KEYCODE_4:				return '4';
		case KeyEvent.KEYCODE_5:				return '5';
		case KeyEvent.KEYCODE_6:				return '6';
		case KeyEvent.KEYCODE_7:				return '7';
		case KeyEvent.KEYCODE_8:				return '8';
		case KeyEvent.KEYCODE_9:				return '9';

		case KeyEvent.KEYCODE_A:				return 'A';
		case KeyEvent.KEYCODE_B:				return 'B';
		case KeyEvent.KEYCODE_C:				return 'C';
		case KeyEvent.KEYCODE_D:				return 'D';
		case KeyEvent.KEYCODE_E:				return 'E';
		case KeyEvent.KEYCODE_F:				return 'F';
		case KeyEvent.KEYCODE_G:				return 'G';
		case KeyEvent.KEYCODE_H:				return 'H';
		case KeyEvent.KEYCODE_I:				return 'I';
		case KeyEvent.KEYCODE_J:				return 'J';
		case KeyEvent.KEYCODE_K:				return 'K';
		case KeyEvent.KEYCODE_L:				return 'L';
		case KeyEvent.KEYCODE_M:				return 'M';
		case KeyEvent.KEYCODE_N:				return 'N';
		case KeyEvent.KEYCODE_O:				return 'O';
		case KeyEvent.KEYCODE_P:				return 'P';
		case KeyEvent.KEYCODE_Q:				return 'Q';
		case KeyEvent.KEYCODE_R:				return 'R';
		case KeyEvent.KEYCODE_S:				return 'S';
		case KeyEvent.KEYCODE_T:				return 'T';
		case KeyEvent.KEYCODE_U:				return 'U';
		case KeyEvent.KEYCODE_V:				return 'V';
		case KeyEvent.KEYCODE_W:				return 'W';
		case KeyEvent.KEYCODE_X:				return 'X';
		case KeyEvent.KEYCODE_Y:				return 'Y';
		case KeyEvent.KEYCODE_Z:				return 'Z';
		
		case KeyEvent.KEYCODE_ENTER:			return 'e';
		case KeyEvent.KEYCODE_SPACE:			return ' ';
		case KeyEvent.KEYCODE_SHIFT_LEFT:
		case KeyEvent.KEYCODE_SHIFT_RIGHT:		return 'c';
		case KeyEvent.KEYCODE_ALT_LEFT:
		case KeyEvent.KEYCODE_ALT_RIGHT:		return 's';

//		case KeyEvent.KEYCODE_MENU:				return 'm';
//		case KeyEvent.KEYCODE_BACK:				return 'k';
		case KeyEvent.KEYCODE_CALL:				return 'e';
		case KeyEvent.KEYCODE_CAMERA:			return ' ';
		}
		return 0;
	}
	@Override
    public boolean onKey(View v, int keyCode, KeyEvent event)
	{
		final char k = TranslateKey(keyCode);
		if(k == 0)
			return false;
		Emulator.the.OnKey(k, event.getAction() == KeyEvent.ACTION_DOWN, event.isShiftPressed(), event.isAltPressed());
		return true;
    }
}
