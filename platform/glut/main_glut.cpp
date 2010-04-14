/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../../std.h"
#include "../platform.h"
#include "../../ui/dialogs.h"

#ifdef USE_GLUT

#include <GL/glut.h>
#include <ctype.h>

xUi::eManager* ui_manager = NULL;

namespace xPlatform
{

void InitSound();
void DoneSound();
void OnLoopSound();

void DrawGL(int w, int h, void* data, dword* data_ui);

static int window = -1;
static int w = 1, h = 1;

static void OnResizeWindow(int Width, int Height)
{
	if(Height == 0)
		Height = 1;
	w = Width;
	h = Height;
}

static void Draw()
{
	DrawGL(w, h, Handler()->VideoData(), ui_manager->VideoData());
	glutSwapBuffers();
}

static void OnDraw() { Draw(); }
static void OnIdle()
{
	Handler()->OnLoop();
	ui_manager->Update();
	glutPostRedisplay();
	OnLoopSound();
}

static void TranslateKey(byte& _key, dword& _flags)
{
	// translate ASCII to speccy keys
	switch(_key)
	{
	case '\r':	_key = 'e';		break; // enter
	case '\b': // backspace
		_key = '0';
		_flags |= KF_SHIFT;
		break;
	case '\"': // double quote
		_key = 'P';
		_flags |= KF_ALT;
		_flags &= ~KF_SHIFT;
		break;
	case '\t': // convert tab to ss + cs
		_key = '\0';
		_flags |= KF_ALT;
		_flags |= KF_SHIFT;
		break;

	case '!':	_key = '1';		break;
	case '@':	_key = '2';		break;
	case '#':	_key = '3';		break;
	case '$':	_key = '4';		break;
	case '%':	_key = '5';		break;
	case '^':	_key = '6';		break;
	case '&':	_key = '7';		break;
	case '*':	_key = '8';		break;
	case '(':	_key = '9';		break;
	case ')':	_key = '0';		break;
	}
}

static void OnKeyDown(unsigned char _key, int x, int y)
{
	dword flags = KF_DOWN;
	int m = glutGetModifiers();
	if(m&GLUT_ACTIVE_SHIFT)
		flags |= KF_SHIFT;
	if(m&GLUT_ACTIVE_CTRL)
		flags |= KF_CTRL;
	if(m&GLUT_ACTIVE_ALT)
		flags |= KF_ALT;
	byte key = toupper(_key);
	TranslateKey(key, flags);
	if(!ui_manager->Focused())
		Handler()->OnKey(key, flags);
	ui_manager->OnKey(key);
}

static void OnKeyUp(unsigned char _key, int x, int y)
{
	dword flags = 0;
	byte key = toupper(_key);
	TranslateKey(key, flags);
	if(!ui_manager->Focused())
		Handler()->OnKey(key, 0);
	ui_manager->OnKey('\0');
}

void Done();
bool Init(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(320 * 2, 240 * 2);
	glutInitWindowPosition(100, 100);
	Handler()->OnInit();
	window = glutCreateWindow(Handler()->WindowCaption());
	glutDisplayFunc(&OnDraw);
//	glutFullScreen();
	glutIdleFunc(&OnIdle);
	glutReshapeFunc(&OnResizeWindow);
	glutIgnoreKeyRepeat(true);
	glutKeyboardFunc(&OnKeyDown);
	glutKeyboardUpFunc(&OnKeyUp);
	InitSound();
	atexit(Done);
	ui_manager = new xUi::eManager("/*");
	ui_manager->Init();
	return true;
}
void Loop()
{
	glutMainLoop(); // app really never exit from this function
}
void Done()
{
	DoneSound();
	Handler()->OnDone();
	SAFE_DELETE(ui_manager);
}

}
//namespace xPlatform

#ifdef _WINDOWS
#include <windows.h>
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
#else//_WINDOWS
int main(int argc, char* argv[])
{
#endif//_WINDOWS
	if(!xPlatform::Init(argc, (char**)argv))
		return -1;
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

#endif//USE_GLUT
