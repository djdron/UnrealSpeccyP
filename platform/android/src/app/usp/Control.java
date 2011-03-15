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

import android.content.Context;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.widget.ImageView;

public class Control extends ImageView
{
	static final int SIZE = 160;
	static final int THRESHOLD = 20;
	public Control(Context context)
	{
		super(context);
		setImageResource(R.drawable.control);
		setFocusable(true);
		setFocusableInTouchMode(true);
		setScaleType(ScaleType.CENTER);
		requestFocus();
	}
	protected void onMeasure(int w, int h)
	{
		setMeasuredDimension(SIZE, SIZE);
	}
	static private char TranslateKey(int keyCode)
	{
		switch(keyCode)
		{
		case KeyEvent.KEYCODE_DPAD_LEFT:		return 'l';
		case KeyEvent.KEYCODE_DPAD_RIGHT:		return 'r';
		case KeyEvent.KEYCODE_DPAD_UP:			return 'u';
		case KeyEvent.KEYCODE_DPAD_DOWN:		return 'd';
		case KeyEvent.KEYCODE_DPAD_CENTER:		return 'f';
		case KeyEvent.KEYCODE_MENU:				return 'm';
		case KeyEvent.KEYCODE_CALL:				return 'e';
		case KeyEvent.KEYCODE_BACK:				return 'k';
		}
		return 0;
	}
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		final char k = TranslateKey(keyCode);
		if(k == 0)
		{
			super.onKeyDown(keyCode, event);
			return false;
		}
		Emulator.the.OnKey(k, true);
		return true;
	}
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		final char k = TranslateKey(keyCode);
		if(k == 0)
		{
			super.onKeyUp(keyCode, event);
			return false;
		}
		Emulator.the.OnKey(k, false);
		return true;
	}
	public boolean onTouchEvent(MotionEvent event)
	{
		final int a = event.getAction();
		if(a == MotionEvent.ACTION_UP || a == MotionEvent.ACTION_CANCEL)
		{
			Emulator.the.OnKey('r', false);
			Emulator.the.OnKey('l', false);
			Emulator.the.OnKey('u', false);
			Emulator.the.OnKey('d', false);
			Emulator.the.OnKey('f', false);
			return true;
		}
		final float x = event.getX() - SIZE/2;
		final float y = event.getY() - SIZE/2;
		if(Math.abs(x) < THRESHOLD && Math.abs(y) < THRESHOLD)
		{
			Emulator.the.OnKey('f', true);
		}
		if(x < -THRESHOLD)
		{
			Emulator.the.OnKey('r', false);
			Emulator.the.OnKey('l', true);
		}
		else if(x > +THRESHOLD)
		{
			Emulator.the.OnKey('l', false);
			Emulator.the.OnKey('r', true);
		}
		if(y < -THRESHOLD)
		{
			Emulator.the.OnKey('d', false);
			Emulator.the.OnKey('u', true);
		}
		else if(y > +THRESHOLD)
		{
			Emulator.the.OnKey('u', false);
			Emulator.the.OnKey('d', true);
		}
		return true;
	}
}
