/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2021 SMT, Dexus, Alone Coder, deathsoft, djdron, scor, 3dEyes

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

#include "../platform.h"
#include "../../tools/options.h"
#include "../../options_common.h"

#include <Joystick.h>

namespace xPlatform
{

enum { AXES_COUNT = 8 };
enum ePadButton
{
	PB_A, PB_B, PB_X, PB_Y,
	PB_LTRIGGER, PB_RTRIGGER,
	PB_LSHIFT, PB_RSHIFT,
	PB_SELECT, PB_START,
	PB_STICK1, PB_STICK2,
	PB_UP, PB_DOWN,
	PB_LEFT, PB_RIGHT,
	PB_SYSTEM, PB_COUNT
};
enum eHatButton
{
	HAT_CENTERED, HAT_UP,
	HAT_RIGHT_UP, HAT_RIGHT,
	HAT_RIGHT_DOWN, HAT_DOWN,
	HAT_LEFT_DOWN, HAT_LEFT,
	HAT_LEFT_UP, HAT_COUNT
};

static BJoystick *joystick = NULL;
static uint32 lastButtonValues = 0;
static uint32 lastHatBitValues = 0;
static float lastAxisValues[AXES_COUNT];

static uint32 HatToBitValues(uint8 hat)
{
	uint32 bits[] = { 0x00, 0x01, 0x03, 0x02, 0x06, 0x04, 0x0A, 0x08, 0x09 };
	if (hat > 8)
		return 0;
	return bits[hat];
}

static void ProcessKey(bool state, bool state_prev, byte key)
{
	if(state == state_prev)
		return;
	if(state)
		Handler()->OnKey(key, KF_DOWN | OpJoyKeyFlags());
	else
		Handler()->OnKey(key, OpJoyKeyFlags());
}

static void ProcessAxis(float value, float lastValue, byte key1, byte key2)
{
	bool b0 = lastValue > 0.5f;
	bool b1 = value > 0.5f;
	ProcessKey(b1, b0, key1);
	b0 = lastValue < -0.5f;
	b1 = value < -0.5f;
	ProcessKey(b1, b0, key2);
}

bool InitJoystick()
{
	joystick = new BJoystick();
	if (joystick->CountDevices() > 0) {
		char devName[B_OS_NAME_LENGTH];
		memset(devName, 0, B_OS_NAME_LENGTH);
		joystick->GetDeviceName(0, devName);
		if (joystick->Open(devName) != B_ERROR)
			return true;
	}
	delete joystick;
	joystick = NULL;
	return false;
}

void DoneJoystick()
{
	if (joystick != NULL)
		delete joystick;
}

bool CheckJoystick()
{
	return (joystick != NULL);
}

void ProcessJoystick()
{
	if (joystick == NULL)
		return;

	joystick->Update();

	uint32 buttonValues = joystick->ButtonValues();
	for (int buttonIdx = 0; buttonIdx < joystick->CountButtons(); buttonIdx++) {
		bool buttonValue = buttonValues & (1 << buttonIdx);
		bool lastButtonValue = lastButtonValues & (1 << buttonIdx);
		if (buttonValue != lastButtonValue) {
			switch (buttonIdx) {
				case PB_UP:
					ProcessKey(buttonValue, lastButtonValue, 'u');
					break;
				case PB_DOWN:
					ProcessKey(buttonValue, lastButtonValue, 'd');
					break;
				case PB_LEFT:
					ProcessKey(buttonValue, lastButtonValue, 'l');
					break;
				case PB_RIGHT:
					ProcessKey(buttonValue, lastButtonValue, 'r');
					break;
				case PB_A:
					ProcessKey(buttonValue, lastButtonValue, 'f');
					break;
				case PB_B:
					ProcessKey(buttonValue, lastButtonValue, 'e');
					break;
				case PB_X:
					ProcessKey(buttonValue, lastButtonValue, '1');
					break;
				case PB_Y:
					ProcessKey(buttonValue, lastButtonValue, ' ');
					break;
				case PB_LSHIFT:
					ProcessKey(buttonValue, lastButtonValue, 'm');
					break;
				case PB_RSHIFT:
					ProcessKey(buttonValue, lastButtonValue, 'k');
					break;
				case PB_SELECT:
					Handler()->OnAction(A_RESET);
					break;
				case PB_START:
					if(!buttonValue) {
						using namespace xOptions;
						eOptionB* o = eOptionB::Find("pause");
						SAFE_CALL(o)->Change();
					}
					break;
			}
		}
	}
	lastButtonValues = buttonValues;
	
	int hatsCount = joystick->CountHats();
	uint8 hatsValues[hatsCount];
	joystick->GetHatValues(hatsValues, 0);
	
	if (hatsCount > 0) {
		uint32 hatBitValues = HatToBitValues(hatsValues[0]);
		for (int hatButtonIdx = 0; hatButtonIdx < joystick->CountButtons(); hatButtonIdx++) {
			bool hatValue = hatBitValues & (1 << hatButtonIdx);
			bool lastHatValue = lastHatBitValues & (1 << hatButtonIdx);
			if (hatValue != lastHatValue) {
				switch (hatButtonIdx) {
					case 0:
						ProcessKey(hatValue, lastHatValue, 'u');
						break;
					case 1:
						ProcessKey(hatValue, lastHatValue, 'r');
						break;
					case 2:
						ProcessKey(hatValue, lastHatValue, 'd');
						break;
					case 3:
						ProcessKey(hatValue, lastHatValue, 'l');
						break;
				}
			}
		}
		lastHatBitValues = hatBitValues;
	}

	int16 axes[joystick->CountAxes()];
	joystick->GetAxisValues(axes);
	for (int axesIdx = 0; axesIdx < joystick->CountAxes() && axesIdx < AXES_COUNT; axesIdx++) {
		double value = 0;
		if (axes[axesIdx] >= 0)
			value = axes[axesIdx] / 32767.0;
		else
			value = axes[axesIdx] / 32768.0;
		
		if (axesIdx == 0 || axesIdx == 3 || axesIdx == 6)
			ProcessAxis(value, lastAxisValues[axesIdx], 'r', 'l');

		if (axesIdx == 1 || axesIdx == 4 || axesIdx == 7)
			ProcessAxis(value, lastAxisValues[axesIdx], 'd', 'u');
		lastAxisValues[axesIdx] = value;
	}
}

}
