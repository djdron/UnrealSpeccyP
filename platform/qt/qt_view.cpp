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

#include <QAudioOutput>
#include <QPainter>
#include "qt_view.h"
#include "../platform.h"
#include "../../tools/options.h"
#include "../../options_common.h"
#include "../../ui/ui.h"

namespace xPlatform
{

#ifdef Q_WS_S60
static struct eOptionSkipFrames : public xOptions::eOptionInt
{
	eOptionSkipFrames() { Set(1); }
	virtual const char* Name() const { return "skip frames"; }
	virtual const char** Values() const
	{
		static const char* values[] = { "none", "2", "4", NULL };
		return values;
	}
	virtual int Order() const { return 3; }
} op_skip_frames;
#endif//Q_WS_S60

//=============================================================================
//	eView::eView
//-----------------------------------------------------------------------------
eView::eView(QWidget* parent) : QWidget(parent), screen(320, 240, QImage::Format_RGB32)
{
	setAttribute(Qt::WA_NoSystemBackground, true);
	OpLastFile("/");
	Handler()->OnInit();
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
	Handler()->OnDone();
}
//=============================================================================
//	eView::sizeHint
//-----------------------------------------------------------------------------
QSize eView::sizeHint() const { return screen.size(); }
QSize eView::minimumSizeHint() const { return screen.size(); }
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
	QPoint p((width() - screen.width())/2, (height() - screen.height())/2);
	painter.drawImage(p, screen);
}

//=============================================================================
//	eView::timerEvent
//-----------------------------------------------------------------------------
void eView::timerEvent(QTimerEvent* event)
{
	Handler()->OnLoop();
	UpdateSound();
#ifdef Q_WS_S60
	static int frame = 0;
	int s = op_skip_frames ? (1 << (int)op_skip_frames + 1) : 1;
	if(++frame % s == 0)
#endif//Q_WS_S60
	{
		UpdateScreen(screen.bits());
		update();
	}
}

}
//namespace xPlatform

#endif//USE_QT
