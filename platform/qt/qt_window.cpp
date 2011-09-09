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

#include "qt_window.h"
#include "qt_control.h"
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
//	eWindow::eWindow
//-----------------------------------------------------------------------------
eWindow::eWindow(QWidget* parent) : QMainWindow(parent)
{
	setWindowTitle(xPlatform::Handler()->WindowCaption());

#ifndef Q_WS_S60
	QMenu* menu = menuBar()->addMenu(tr("&File"));
#else//Q_WS_S60
	QMenuBar* menu = menuBar();
#endif//Q_WS_S60

	QAction* aopen = new QAction(tr("&Open..."), this);
	aopen->setShortcuts(QKeySequence::Open);
	aopen->setStatusTip(tr("Open file"));
	connect(aopen, SIGNAL(triggered()), this, SLOT(OnOpenFile()));
	menu->addAction(aopen);

	QAction* areset = new QAction(tr("&Reset..."), this);
	connect(areset, SIGNAL(triggered()), this, SLOT(OnReset()));
	menu->addAction(areset);

#ifndef Q_WS_S60
	QAction* aquit = new QAction(tr("E&xit"), this);
	aquit->setShortcuts(QKeySequence::Quit);
	connect(aquit, SIGNAL(triggered()), this, SLOT(close()));
	menu->addAction(aquit);
#endif//Q_WS_S60

	QWidget* w = new QWidget;
	QLayout* l = new QVBoxLayout(w);
	l->setMargin(0);
	l->setSpacing(0);
	l->addWidget(view = new eView);
	l->addWidget(control = new eControl);
	setCentralWidget(w);
	control->setFocus();
}
//=============================================================================
//	eWindow::OnOpenFile
//-----------------------------------------------------------------------------
void eWindow::OnOpenFile()
{
	using namespace xPlatform;
	QString name = QFileDialog::getOpenFileName(this, tr("Open file"), OpLastFolder(),
		tr(	"Supported files (*.sna *.z80 *.trd *.scl *.tap *.csw *.tzx *.zip);;"
			"All files (*.*);;"
			"Snapshot files (*.sna *.z80);;"
			"Disk images (*.trd *.scl);;"
			"Tape files (*.tap *.csw *.tzx);;"
			"ZIP archives (*.zip)"
			));

	if(!name.isEmpty())
		Handler()->OnOpenFile(name.toUtf8().data());
}
//=============================================================================
//	eWindow::OnOpenFile
//-----------------------------------------------------------------------------
void eWindow::OnReset()
{
	xPlatform::Handler()->OnAction(xPlatform::A_RESET);
}

#endif//USE_QT
