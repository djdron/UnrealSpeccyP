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

#ifdef USE_QT

#include <QtGui>
#include <QAudioOutput>

#include "window.h"
#include "qt_sound.h"
#include "../platform.h"
#include "../../options_common.h"
#include "../../ui/ui.h"
#include "../../tools/options.h"

static xOptions::eOption<int>* op_sound = NULL;
static int OpSound() { return op_sound ? *op_sound : (int)xPlatform::S_AY; }

//=============================================================================
//	Window::Window
//-----------------------------------------------------------------------------
Window::Window(QWidget* parent) : QWidget(parent), screen(320, 240, QImage::Format_RGB32)
{
	using namespace xPlatform;
	OpLastFile("/");
	Handler()->OnInit();
	op_sound = xOptions::eOption<int>::Find("sound");
	setWindowTitle(Handler()->WindowCaption());
	setFixedSize(screen.size());
	startTimer(10);
	QAudioFormat fmt;
	fmt.setFrequency(44100);
	fmt.setChannels(2);
	fmt.setSampleSize(16);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::SignedInt);
	audio_stream = new eAudioStream(this);
	audio = new QAudioOutput(fmt, this);
	audio->start(audio_stream);
}

//=============================================================================
//	Window::~Window
//-----------------------------------------------------------------------------
Window::~Window()
{
	audio->stop();
	xPlatform::Handler()->OnDone();
}

//=============================================================================
//	eCachedColors
//-----------------------------------------------------------------------------
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
			items[c] = qRgb(r, g, b);
		}
	}
	dword items[16];
}
color_cache;

//=============================================================================
//	Window::UpdateScreen
//-----------------------------------------------------------------------------
void Window::UpdateScreen(uchar* _scr) const
{
	using namespace xPlatform;
	byte* data = (byte*)Handler()->VideoData();
	dword* p = (dword*)_scr;
#ifdef USE_UI
	dword* data_ui = (dword*)Handler()->VideoDataUI();
	if(data_ui)
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				xUi::eRGBAColor c_ui = *data_ui++;
				xUi::eRGBAColor c = color_cache.items[*data++];
				*p++ = qRgb((c.b >> c_ui.a) + c_ui.r, (c.g >> c_ui.a) + c_ui.g, (c.r >> c_ui.a) + c_ui.b);
			}
		}
	}
	else
#endif//USE_UI
	{
		for(int y = 0; y < 240; ++y)
		{
			for(int x = 0; x < 320; ++x)
			{
				*p++ = color_cache.items[*data++];
			}
		}
	}
}

void Window::UpdateSound()
{
	using namespace xPlatform;
	for(int i = Handler()->AudioSources(); --i >= 0;)
	{
		dword size = Handler()->AudioDataReady(i);
		if(i == OpSound() && !Handler()->FullSpeed())
		{
			audio_stream->Fill(Handler()->AudioData(i), size);
		}
		Handler()->AudioDataUse(i, size);
	}
	static bool video_paused = false;
	bool video_paused_new = audio_stream->Ready() > 44100*2*2/50;
	if(video_paused_new != video_paused)
	{
		video_paused = video_paused_new;
		Handler()->VideoPaused(video_paused);
	}
	if(audio->state() != QAudio::ActiveState)
	{
		audio->start(audio_stream);
	}
}

//=============================================================================
//	Window::paintEvent
//-----------------------------------------------------------------------------
void Window::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawImage(QPointF(0, 0), screen);
}

//=============================================================================
//	Window::timerEvent
//-----------------------------------------------------------------------------
void Window::timerEvent(QTimerEvent* event)
{
	xPlatform::Handler()->OnLoop();
	UpdateScreen(screen.bits());
	UpdateSound();
	update();
}
//=============================================================================
//	Window::keyPressEvent
//-----------------------------------------------------------------------------
void Window::keyPressEvent(QKeyEvent* event)
{
	if(event->isAutoRepeat())
	{
		event->ignore();
		return;
	}
	int key = event->key();
	using namespace xPlatform;
	dword flags = KF_DOWN|OpJoyKeyFlags();
	if(event->modifiers()&Qt::ALT)		flags |= KF_ALT;
	if(event->modifiers()&Qt::SHIFT)	flags |= KF_SHIFT;
	TranslateKey(key, flags);
	xPlatform::Handler()->OnKey(key, flags);
	key > 0 ? event->accept() : event->ignore();
}
//=============================================================================
//	Window::keyReleaseEvent
//-----------------------------------------------------------------------------
void Window::keyReleaseEvent(QKeyEvent* event)
{
	if(event->isAutoRepeat())
	{
		event->ignore();
		return;
	}
	int key = event->key();
	using namespace xPlatform;
	dword flags = 0;
	if(event->modifiers()&Qt::ALT)		flags |= KF_ALT;
	if(event->modifiers()&Qt::SHIFT)	flags |= KF_SHIFT;
	TranslateKey(key, flags);
	Handler()->OnKey(key, OpJoyKeyFlags());
	key > 0 ? event->accept() : event->ignore();
}
//=============================================================================
//	Window::keyReleaseEvent
//-----------------------------------------------------------------------------
void Window::TranslateKey(int& key, dword& flags) const
{
	using namespace xPlatform;
	switch(key)
	{
	case Qt::Key_Shift:		key = 'c';	break;
	case Qt::Key_Alt:		key = 's';	break;
	case Qt::Key_Return:	key = 'e';	break;
	case Qt::Key_Tab:
		key = '\0';
		flags |= KF_ALT;
		flags |= KF_SHIFT;
		break;
	case Qt::Key_Backspace:
		key = '0';
		flags |= KF_SHIFT;
		break;
	case Qt::Key_Left:		key = 'l';	break;
	case Qt::Key_Right:		key = 'r';	break;
	case Qt::Key_Up:		key = 'u';	break;
	case Qt::Key_Down:		key = 'd';	break;
	case Qt::Key_Control:	key = 'f';	flags &= ~KF_CTRL; break;
	case '!':	key = '1';		break;
	case '@':	key = '2';		break;
	case '#':	key = '3';		break;
	case '$':	key = '4';		break;
	case '%':	key = '5';		break;
	case '^':	key = '6';		break;
	case '&':	key = '7';		break;
	case '*':	key = '8';		break;
	case '(':	key = '9';		break;
	case ')':	key = '0';		break;
	case '~':
	case '`':	key = 'm';		break;
	case '\\':	key = 'k';		break;
	case ']':	key = 'p';		break;
	case '\'':
		key = '7';
		flags |= KF_ALT;
		break;
	case '\"':
		key = 'P';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case '<':
		key = 'R';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case '>':
		key = 'T';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case ',':
		key = 'N';
		flags |= KF_ALT;
		break;
	case '.':
		key = 'M';
		flags |= KF_ALT;
		break;
	case ';':
		key = 'O';
		flags |= KF_ALT;
		break;
	case ':':
		key = 'Z';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case '/':
		key = 'V';
		flags |= KF_ALT;
		break;
	case '?':
		key = 'C';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case '-':
		key = 'J';
		flags |= KF_ALT;
		break;
	case '_':
		key = '0';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	case '=':
		key = 'L';
		flags |= KF_ALT;
		break;
	case '+':
		key = 'K';
		flags &= ~KF_SHIFT;
		flags |= KF_ALT;
		break;
	}
	if(key > 255 || key < 32)
		key = 0;
}

#endif//USE_QT
