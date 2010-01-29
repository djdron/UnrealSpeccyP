#include "../../std.h"

#include "../platform.h"

#ifdef USE_GLUT

#include <GL/glut.h>
#include <ctype.h>

namespace xPlatform
{

void InitSound();
void DoneSound();
void OnLoopSound();

void DrawGL(void* data);

static int window = -1;

static void OnResizeWindow(int Width, int Height)
{
	if(Height == 0)
		Height = 1;
	glViewport(0, 0, Width, Height);
}

static void Draw()
{
	DrawGL(Handler()->VideoData());
	glutSwapBuffers();
}
static void OnDraw() { Draw(); }
static void OnIdle()
{
	Handler()->OnLoop();
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
	Handler()->OnKey(key, flags);
}
static void OnKeyUp(unsigned char _key, int x, int y)
{
	dword flags = 0;
	byte key = toupper(_key);
	TranslateKey(key, flags);
	Handler()->OnKey(key, 0);
}

void Done();
bool Init(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(320, 240);
	glutInitWindowPosition(100, 100);
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
	return true;
}
void Loop()
{
	glutMainLoop(); // app really never exit from this function
}
void Done()
{
	DoneSound();
}

}
//namespace xPlatform

int main(int argc, char* argv[])
{
	if(!xPlatform::Init(argc, argv))
		return -1;
	xPlatform::Loop();
	xPlatform::Done();
	return 0;
}

#endif//USE_GLUT
