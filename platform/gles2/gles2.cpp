/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#ifdef USE_GLES2

#include "gles2_sprite.h"
#include "gles2.h"
#include "../platform.h"
#include "../../tools/options.h"
#include "../../ui/ui.h"

namespace xPlatform
{

static struct eOptionZoom : public xOptions::eOptionInt
{
	eOptionZoom() { Set(2); }
	virtual const char* Name() const { return "zoom"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "fill screen", "small border", "no border", NULL };
		return values;
	}
	virtual void Change(bool next = true)
	{
		eOptionInt::Change(0, 4, next);
	}
	virtual int Order() const { return 35; }
	float Zoom() const
	{
		switch(*this)
		{
		case 2: return 300.0f/256.0f;
		case 3: return 320.0f/256.0f;
		default: return 1.0f;
		}
	}
} op_zoom;

float OpZoom() { return op_zoom.Zoom(); }

static struct eOptionFiltering : public xOptions::eOptionBool
{
	eOptionFiltering() { Set(true); }
	virtual const char* Name() const { return "filtering"; }
	virtual int Order() const { return 36; }
} op_filtering;

static struct eOptionBlackAndWhite : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "black and white"; }
	virtual int Order() const { return 37; }
} op_black_and_white;

static struct eOptionGigaScreen : public xOptions::eOptionBool
{
	virtual const char* Name() const { return "gigascreen"; }
	virtual int Order() const { return 38; }
} op_gigascreen;


static struct eCachedColors
{
	eCachedColors()
	{
		const byte brightness = 200;
		const byte bright_intensity = 55;
		for(int c = 0; c < 16; ++c)
		{
			byte i = c&8 ? brightness + bright_intensity : brightness;
			byte b = c&1 ? i : 0;
			byte r = c&2 ? i : 0;
			byte g = c&4 ? i : 0;
			items[c] = BGR565(r, g, b);
		}
	}
	static inline word BGR565(byte r, byte g, byte b) { return ((r&~7) << 8)|((g&~3) << 3)|(b >> 3); }
	word items[16];
}
color_cache;

static const char* fragment_shader_bw =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"uniform mediump vec4 u_color;						    \n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"	gl_FragColor *= u_color;						    \n"
	"   mediump float l = dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114)); \n"
	"	gl_FragColor.rgb = vec3(l, l, l);				    \n"
	"}														\n";

static const char* fragment_shader_giga =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"uniform sampler2D u_texture2;							\n"
	"uniform mediump vec4 u_color;						    \n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"	gl_FragColor += texture2D(u_texture2, v_texcoord);	\n"
	"	gl_FragColor *= u_color*0.5;					    \n"
	"}														\n";

static const char* fragment_shader_giga_bw =
	"varying mediump vec2 v_texcoord;						\n"
	"uniform sampler2D u_texture;							\n"
	"uniform sampler2D u_texture2;							\n"
	"uniform mediump vec4 u_color;						    \n"
	"void main()											\n"
	"{														\n"
	"	gl_FragColor = texture2D(u_texture, v_texcoord);	\n"
	"	gl_FragColor += texture2D(u_texture2, v_texcoord);	\n"
	"	gl_FragColor *= u_color*0.5;					    \n"
	"   mediump float l = dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114)); \n"
	"	gl_FragColor.rgb = vec3(l, l, l);				    \n"
	"}														\n";


class eGLES2Impl : public eGLES2
{
public:
	eGLES2Impl();
	virtual ~eGLES2Impl();
	virtual void Draw(const ePoint& pos, const ePoint& size);

protected:

	enum { WIDTH = 320, HEIGHT = 240, TEX_WIDTH = 512, TEX_HEIGHT = 256 };
	byte texture_buffer[2][TEX_WIDTH*TEX_HEIGHT*2]; //2 buffers of R5G6B5

	eGLES2Sprite* sprite_screen;
	eGLES2Sprite* sprite_screen_bw;
	eGLES2Sprite* sprite_gigascreen;
	eGLES2Sprite* sprite_gigascreen_bw;
	GLuint texture[2];
	int	texture_current;
	void UpdateScreenTexture();

	int video_frame_last;

#ifdef USE_UI
	void UpdateUiTexture();
	GLuint texture_ui;
	byte texture_buffer_ui[TEX_WIDTH*TEX_HEIGHT*4]; //buffer of R8G8B8A8
#endif//USE_UI
};

eGLES2Impl::eGLES2Impl() : texture_current(0), video_frame_last(-1)
{
	sprite_screen = new eGLES2Sprite(ePoint(WIDTH, HEIGHT));
	sprite_screen_bw = new eGLES2Sprite(ePoint(WIDTH, HEIGHT), NULL, fragment_shader_bw);
	sprite_gigascreen = new eGLES2Sprite(ePoint(WIDTH, HEIGHT), NULL, fragment_shader_giga);
	sprite_gigascreen_bw = new eGLES2Sprite(ePoint(WIDTH, HEIGHT), NULL, fragment_shader_giga_bw);

	memset(texture_buffer, 0, sizeof(texture_buffer));

	glGenTextures(2, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer[0]);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer[1]);

#ifdef USE_UI
	memset(texture_buffer_ui, 0, sizeof(texture_buffer_ui));
	glGenTextures(1, &texture_ui);
	glBindTexture(GL_TEXTURE_2D, texture_ui);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_ui);
#endif//USE_UI

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_DITHER);
}

eGLES2Impl::~eGLES2Impl()
{
	SAFE_DELETE(sprite_screen);
	SAFE_DELETE(sprite_screen_bw);
	SAFE_DELETE(sprite_gigascreen);
	SAFE_DELETE(sprite_gigascreen_bw);
	glDeleteTextures(2, texture);
#ifdef USE_UI
	glDeleteTextures(1, &texture_ui);
#endif//USE_UI
}

void eGLES2Impl::Draw(const ePoint& pos, const ePoint& size)
{
	bool filtering = op_filtering;
	float sx, sy;
	switch(op_zoom)
	{
	case 0:
	    filtering = false;
	    sx = ((float)WIDTH) / size.x;
	    sy = ((float)HEIGHT) / size.y;
		break;
	default:
		GetScaleWithAspectRatio43(&sx, &sy, size.x, size.y);
		break;
	}

	glClear(GL_COLOR_BUFFER_BIT);

	if(video_frame_last != Handler()->VideoFrame())
	{
		video_frame_last = Handler()->VideoFrame();
		texture_current = 1 - texture_current;
		UpdateScreenTexture();
		glActiveTexture(GL_TEXTURE0 + texture_current);
		glBindTexture(GL_TEXTURE_2D, texture[texture_current]);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, texture_buffer[texture_current]);
	}
	float z = OpZoom();
	if(op_gigascreen)
		(op_black_and_white ? sprite_gigascreen_bw : sprite_gigascreen)->Draw2(texture[texture_current], texture[1 - texture_current], pos, size, 1.0f, sx*z, sy*z, filtering);
	else
		(op_black_and_white ? sprite_screen_bw : sprite_screen)->Draw(texture[texture_current], pos, size, 1.0f, sx*z, sy*z, filtering);

#ifdef USE_UI
	void* data_ui = Handler()->VideoDataUI();
	if(data_ui)
	{
		float sx_ui, sy_ui;
		GetScaleWithAspectRatio43(&sx_ui, &sy_ui, size.x, size.y);
		UpdateUiTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_ui);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, texture_buffer_ui);
		sprite_screen->Draw(texture_ui, pos, size, 0.99f, sx_ui, sy_ui); // force alpha blend
	}
#endif//USE_UI
//	glFlush();
}

void eGLES2Impl::UpdateScreenTexture()
{
	byte* src = (byte*)Handler()->VideoData();
	word* dst = (word*)texture_buffer[texture_current];
	for(int i = WIDTH*HEIGHT; --i >= 0;)
	{
		*dst++ = color_cache.items[*src++];
	}
}

#ifdef USE_UI
void eGLES2Impl::UpdateUiTexture()
{
	byte* src = (byte*)Handler()->VideoDataUI();
	dword* dst = (dword*)texture_buffer_ui;
	for(int i = WIDTH*HEIGHT; --i >= 0;)
	{
		*dst++ = xUi::palette[*src++].rgba;
	}
}
#endif//USE_UI

eGLES2* eGLES2::Create() { return new eGLES2Impl; }

}
//namespace xPlatform

#endif//USE_GLES2
