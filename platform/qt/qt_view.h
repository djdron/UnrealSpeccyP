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

#ifndef QT_VIEW_H
#define QT_VIEW_H

#include <QWidget>
#include "qt_sound.h"

#pragma once

class QAudioOutput;

namespace xPlatform
{

//=============================================================================
//	eView
//-----------------------------------------------------------------------------
class eView : public QWidget
{
	Q_OBJECT

public:
	eView(QWidget* parent = NULL);
	~eView();

protected:
	virtual void	paintEvent(QPaintEvent* event);
	virtual void	timerEvent(QTimerEvent* event);

	void			UpdateScreen(uchar* data) const;
	void			UpdateSound();

protected:
	QImage			screen;
	QAudioOutput*	audio;
	QIODevice*		stream;
	eAudioBuffer	audio_buffer;
};

}
//namespace xPlatform

#endif//QT_VIEW_H
