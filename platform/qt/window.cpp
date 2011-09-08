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
//	eView::eView
//-----------------------------------------------------------------------------
eView::eView(QWidget* parent) : QWidget(parent), screen(320, 240, QImage::Format_RGB32)
{
	using namespace xPlatform;
	OpLastFile("/");
	Handler()->OnInit();
	op_sound = xOptions::eOption<int>::Find("sound");
	setAttribute(Qt::WA_InputMethodEnabled, false);
	setFocusPolicy(Qt::StrongFocus);
	setFixedSize(screen.size());
	startTimer(10);
	QAudioFormat fmt;
	fmt.setFrequency(44100);
	fmt.setChannels(2);
	fmt.setSampleSize(16);
	fmt.setCodec("audio/pcm");
	fmt.setByteOrder(QAudioFormat::LittleEndian);
	fmt.setSampleType(QAudioFormat::SignedInt);
	audio = new QAudioOutput(fmt, this);
	stream = audio->start();
}

//=============================================================================
//	eView::~eView
//-----------------------------------------------------------------------------
eView::~eView()
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
//	eView::UpdateScreen
//-----------------------------------------------------------------------------
void eView::UpdateScreen(uchar* _scr) const
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
//=============================================================================
//	eView::UpdateSound
//-----------------------------------------------------------------------------
void eView::UpdateSound()
{
	audio_buffer.Update(OpSound());
	if(audio->state() != QAudio::StoppedState)
	{
		qint64 out = stream->write((const char*)audio_buffer.Ptr(), audio_buffer.Ready());
		audio_buffer.Use(out);
	}
}

//=============================================================================
//	eView::paintEvent
//-----------------------------------------------------------------------------
void eView::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.drawImage(QPointF(0, 0), screen);
}

//=============================================================================
//	eView::timerEvent
//-----------------------------------------------------------------------------
void eView::timerEvent(QTimerEvent* event)
{
	xPlatform::Handler()->OnLoop();
	UpdateSound();
#ifdef Q_WS_S60 // skip frames
	static int x = 0;
	if(++x % 5 == 0)
#endif//Q_WS_S60
	{
		UpdateScreen(screen.bits());
		update();
	}
}
//=============================================================================
//	eView::keyPressEvent
//-----------------------------------------------------------------------------
void eView::keyPressEvent(QKeyEvent* event)
{
	if(event->isAutoRepeat())
	{
		event->ignore();
		return;
	}
	using namespace xPlatform;
	int key = 0;
	dword flags = KF_DOWN|OpJoyKeyFlags();
	EventKeyFlags(event, &key, &flags);
	TranslateKey(key, flags);
	xPlatform::Handler()->OnKey(key, flags);
	key > 0 ? event->accept() : event->ignore();
}
//=============================================================================
//	eView::keyReleaseEvent
//-----------------------------------------------------------------------------
void eView::keyReleaseEvent(QKeyEvent* event)
{
	if(event->isAutoRepeat())
	{
		event->ignore();
		return;
	}
	using namespace xPlatform;
	int key = 0;
	dword flags = 0;
	EventKeyFlags(event, &key, &flags);
	TranslateKey(key, flags);
	Handler()->OnKey(key, OpJoyKeyFlags());
	key > 0 ? event->accept() : event->ignore();
}
//=============================================================================
//	eView::EventKeyFlags
//-----------------------------------------------------------------------------
void eView::EventKeyFlags(QKeyEvent* event, int* key, dword* flags) const
{
	*key = event->key();
#ifdef Q_WS_S60
	if((event->nativeModifiers() & 0x3000) != 0x3000) // blue arrow not held down
	{
		int nsc = event->nativeScanCode();
		if(nsc >= 'A' && nsc <= 'Z')
			*key = nsc;
	}
	if((event->nativeModifiers() & 0x2800) == 0x2800) *flags |= xPlatform::KF_ALT; // 'sym' key held down
	if(event->modifiers()&Qt::CTRL)		*flags |= xPlatform::KF_ALT;
#else//Q_WS_S60
	if(event->modifiers()&Qt::ALT)		*flags |= xPlatform::KF_ALT;
#endif//Q_WS_S60
	if(event->modifiers()&Qt::SHIFT)	*flags |= xPlatform::KF_SHIFT;
}
//=============================================================================
//	eView::TranslateKey
//-----------------------------------------------------------------------------
void eView::TranslateKey(int& key, dword& flags) const
{
	using namespace xPlatform;
	switch(key)
	{
	case Qt::Key_Shift:		key = 'c';	break;
	case Qt::Key_Alt:		key = 's';	break;
	case Qt::Key_Return:
	case Qt::Key_Enter:		key = 'e';	break;
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
	case Qt::Key_Select:	key = 'f';	break;
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
	case Qt::Key_Yes:
	case Qt::Key_Escape:
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
	if(key > 127 || key < 32)
		key = 0;
}


//=============================================================================
//	eWindow::eWindow
//-----------------------------------------------------------------------------
eWindow::eWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(xPlatform::Handler()->WindowCaption());
	eView* v = new eView(this);
	setCentralWidget(v);
	v->setFocus();
}

#endif//USE_QT
