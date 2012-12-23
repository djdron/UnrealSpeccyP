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

#ifdef _RPI

#include <bcm_host.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <SDL.h>
#include <assert.h>
#include "../gles2/gles2.h"

namespace xPlatform
{

static uint32_t screen_width = 0;
static uint32_t screen_height = 0;
static EGLDisplay display = NULL;
static EGLSurface surface = NULL;
static EGLContext context = NULL;
static EGL_DISPMANX_WINDOW_T nativewindow;
static eGLES2* gles2 = NULL;
static SDL_Surface* screen = NULL;

bool InitVideo()
{
	bcm_host_init();

    screen = SDL_SetVideoMode(0,0, 32, SDL_SWSURFACE); // hack to make keyboard events work

	// get an EGL display connection
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(display != EGL_NO_DISPLAY);

	// initialize the EGL display connection
	EGLBoolean result = eglInitialize(display, NULL, NULL);
	assert(EGL_FALSE != result);

	// get an appropriate EGL frame buffer configuration
	EGLint num_config;
	EGLConfig config;
	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};
	result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);

	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);

	// create an EGL rendering context
	static const EGLint context_attributes[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
	assert(context != EGL_NO_CONTEXT);

	// create an EGL window surface
	int32_t success = graphics_get_display_size(0, &screen_width, &screen_height);
	assert(success >= 0);

	VC_RECT_T dst_rect;
	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = screen_width;
	dst_rect.height = screen_height;

	VC_RECT_T src_rect;
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = screen_width << 16;
	src_rect.height = screen_height << 16;

	DISPMANX_DISPLAY_HANDLE_T dispman_display = vc_dispmanx_display_open(0);
	DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
	DISPMANX_ELEMENT_HANDLE_T dispman_element = vc_dispmanx_element_add(dispman_update, dispman_display,
	  0, &dst_rect, 0, &src_rect, DISPMANX_PROTECTION_NONE, NULL, NULL, DISPMANX_NO_ROTATE);

	nativewindow.element = dispman_element;
	nativewindow.width = screen_width;
	nativewindow.height = screen_height;
	vc_dispmanx_update_submit_sync(dispman_update);

	surface = eglCreateWindowSurface(display, config, &nativewindow, NULL);
	assert(surface != EGL_NO_SURFACE);

	// connect the context to the surface
	result = eglMakeCurrent(display, surface, surface, context);
	assert(EGL_FALSE != result);

	gles2 = eGLES2::Create();
	return true;
}

void DoneVideo()
{
	if(screen)
		SDL_FreeSurface(screen);
	delete gles2;
	gles2 = NULL;
	// Release OpenGL resources
	eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
	eglDestroySurface( display, surface );
	eglDestroyContext( display, context );
	eglTerminate( display );
}

void UpdateScreen()
{
	gles2->Draw(screen_width, screen_height);
	eglSwapBuffers(display, surface);
}

}
//namespace xPlatform

#endif//_RPI
