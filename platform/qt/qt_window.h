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

#ifndef QT_WINDOW_H
#define QT_WINDOW_H

#include <QMainWindow>

#pragma once

namespace xPlatform
{

class eControl;
class eView;

//=============================================================================
//	eWindow
//-----------------------------------------------------------------------------
class eWindow : public QMainWindow
{
	Q_OBJECT
public:
	eWindow(QWidget* parent = NULL);

private slots:
	void OnOpenFile();
	void OnReset();
	void OnControlToggle();
	void OnSaveState();
	void OnLoadState();
	void OnJoyKempston();
	void OnJoyCursor();
	void OnJoyQAOP();
	void OnJoySinclair();
	void OnSndBeeper();
	void OnSndAY();
	void OnSndTape();
	void OnTapeToggle();
	void OnTapeFast();

#ifdef Q_WS_S60
	void OnSkipNone();
	void OnSkip2();
	void OnSkip4();
	void SetSkipFrames(int sf);
#endif//Q_WS_S60

protected:
	bool eventFilter(QObject* receiver, QEvent* event);

protected:
	eView*		view;
	eControl*	control;
	QLayout*	layout;
};

}
//namespace xPlatform

#endif//QT_WINDOW_H
