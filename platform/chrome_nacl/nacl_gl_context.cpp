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

#ifdef _CHROME_NACL

#include "ppapi/cpp/module.h"
#include "ppapi/cpp/completion_callback.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "nacl_gl_context.h"

namespace xPlatform
{

eGLContext::eGLContext(pp::Instance* instance) : pp::Graphics3DClient(instance), flush_pending(false)
{
	pp::Module* module = pp::Module::Get();
	assert(module);
	gles2_interface = static_cast<const struct PPB_OpenGLES2*>(module->GetBrowserInterface(PPB_OPENGLES2_INTERFACE));
	assert(gles2_interface);
}

eGLContext::~eGLContext()
{
	glSetCurrentContextPPAPI(0);
}

bool eGLContext::MakeCurrent(pp::Instance* instance)
{
	if(!instance)
	{
		glSetCurrentContextPPAPI(0);
		return false;
	}
	if(context.is_null())
	{
		int32_t attribs[] =
		{
			PP_GRAPHICS3DATTRIB_ALPHA_SIZE, 8,
			PP_GRAPHICS3DATTRIB_DEPTH_SIZE, 0,
			PP_GRAPHICS3DATTRIB_STENCIL_SIZE, 0,
			PP_GRAPHICS3DATTRIB_SAMPLES, 0,
			PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS, 0,
			PP_GRAPHICS3DATTRIB_WIDTH, size.width(),
			PP_GRAPHICS3DATTRIB_HEIGHT, size.height(),
			PP_GRAPHICS3DATTRIB_NONE
		};
		context = pp::Graphics3D(instance, pp::Graphics3D(), attribs);
		if(context.is_null())
		{
			glSetCurrentContextPPAPI(0);
			return false;
		}
		instance->BindGraphics(context);
	}
	glSetCurrentContextPPAPI(context.pp_resource());
	return true;
}

void eGLContext::Invalidate()
{
	glSetCurrentContextPPAPI(0);
}

void eGLContext::Resize(const pp::Size& _size)
{
	size = _size;
	if(!context.is_null())
		context.ResizeBuffers(size.width(), size.height());
}

void eGLContext::Flush()
{
	if(FlushPending())
		return;
	FlushPending(true);
	context.SwapBuffers(pp::CompletionCallback(&FlushCallback, this));
}

void eGLContext::FlushCallback(void* data, int32_t result)
{
	static_cast<eGLContext*>(data)->FlushPending(false);
}

}
//namespace xPlatform

#endif//_CHROME_NACL
