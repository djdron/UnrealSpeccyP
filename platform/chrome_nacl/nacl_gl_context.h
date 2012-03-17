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

#include "ppapi/cpp/graphics_3d_client.h"
#include "ppapi/cpp/graphics_3d.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/size.h"

namespace xPlatform
{

class eGLContext : public pp::Graphics3DClient
{
public:
	eGLContext(pp::Instance* instance);
	virtual ~eGLContext();

	bool MakeCurrent(pp::Instance* instance);
	void Flush();
	void Invalidate();
	void Resize(const pp::Size& size);
	const struct PPB_OpenGLES2* gles2() const { return gles2_interface; }
	const PP_Resource gl_context() const { return context.pp_resource(); }
	bool FlushPending() const { return flush_pending; }
	void FlushPending(bool flag) { flush_pending = flag; }

private:
	virtual void Graphics3DContextLost() { assert("Graphics3DContextLost"); }
	static void FlushCallback(void* data, int32_t result);

private:
	const struct PPB_OpenGLES2* gles2_interface;
	pp::Graphics3D context;
	pp::Size size;
	bool flush_pending;
};

}
//namespace xPlatform

#endif//_CHROME_NACL
