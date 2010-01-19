#include "../../std.h"

#include "../platform.h"

#ifdef USE_GLUT

#include <GL/glut.h>
#include <ctype.h>

namespace xPlatform
{

void DrawGL(byte* data);

eHandler* handler = NULL;
static int window = -1;

static void OnResizeWindow(int Width, int Height)
{
	if(Height == 0)
		Height = 1;
	glViewport(0, 0, Width, Height);
}

static void Draw()
{
	DrawGL(handler->DrawData());
	glutSwapBuffers();
}
static void OnDraw() { Draw(); }
static void OnIdle()
{
	handler->OnLoop();
	glutPostRedisplay();
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
	handler->OnKey(toupper(key), flags);
}
static void OnKeyUp(unsigned char key, int x, int y)
{
	handler->OnKey(toupper(key), 0);
}

bool Init(int argc, char* argv[], eHandler* h)
{
	handler = h;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(320, 240);
	glutInitWindowPosition(100, 100);
	window = glutCreateWindow(handler->WindowCaption());
	glutDisplayFunc(&OnDraw);
//	glutFullScreen();
	glutIdleFunc(&OnIdle);
	glutReshapeFunc(&OnResizeWindow);
	glutIgnoreKeyRepeat(true);
	glutKeyboardFunc(&OnKeyDown);
	glutKeyboardUpFunc(&OnKeyUp);
	glutMainLoop(); // app really never exit from this function
	return true;
}
// glut not able to exit from glutMainLoop, so Loop(), Done() never called
void Loop()
{
}
void Done()
{
}

}
//namespace xPlatform

#endif//USE_GLUT
