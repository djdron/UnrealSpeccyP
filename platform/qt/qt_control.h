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

#ifndef QT_CONTROL_H
#define QT_CONTROL_H

#include <QWidget>

#pragma once

class eControl : public QWidget
{
public:
	eControl(QWidget* parent = NULL);
	void			ToggleKeyboard();

protected:
	virtual bool	event (QEvent* event);
	virtual void	paintEvent(QPaintEvent* event);
	virtual void	keyPressEvent(QKeyEvent* event);
	virtual void	keyReleaseEvent(QKeyEvent* event);

	void			EventKeyFlags(QKeyEvent* event, int* key, dword* flags) const;
	void			TranslateKey(int& key, dword& flags) const;
	void			OnTouch(float x, float y, bool down, int pointer_id);

	QImage			keyboard;
	QImage			joystick;
	bool			keyboard_active;
};

#endif//QT_CONTROL_H
