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

static void OnKeyDown(unsigned char key, int x, int y)
{
	dword flags = KF_DOWN;
	int m = glutGetModifiers();
	if(m&GLUT_ACTIVE_SHIFT)
		flags |= KF_SHIFT;
	if(m&GLUT_ACTIVE_CTRL)
		flags |= KF_CTRL;
	if(m&GLUT_ACTIVE_ALT)
		flags |= KF_ALT;
	Handler()->OnKey(toupper(key), flags);
}
static void OnKeyUp(unsigned char key, int x, int y)
{
	Handler()->OnKey(toupper(key), 0);
}

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

#endif//USE_GLUT
