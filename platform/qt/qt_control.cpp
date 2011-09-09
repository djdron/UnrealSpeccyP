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

#include "../../std_types.h"
#include "../platform.h"
#include "../../options_common.h"
#include "../touch_ui/tui_keyboard.h"
#include "../touch_ui/tui_joystick.h"
#include "qt_control.h"

//=============================================================================
//	eControl::eControl
//-----------------------------------------------------------------------------
eControl::eControl(QWidget* parent) : QWidget(parent), keyboard_active(true)
{
	keyboard.load(":/image/keyboard.png");
	joystick.load(":/image/joystick.png");
	setFixedSize(keyboard.size());
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_AcceptTouchEvents);
	setContextMenuPolicy(Qt::NoContextMenu);
}
//=============================================================================
//	eControl::ToggleKeyboard
//-----------------------------------------------------------------------------
void eControl::ToggleKeyboard()
{
	keyboard_active = !keyboard_active;
	update();
}
//=============================================================================
//	eControl::OnTouch
//-----------------------------------------------------------------------------
void eControl::OnTouch(float x, float y, bool down, int pointer_id)
{
	if(keyboard_active)
		xPlatform::OnTouchKey(x, y, down, pointer_id);
	else
		xPlatform::OnTouchJoy(x, y, down, pointer_id);
}
//=============================================================================
//	eControl::paintEvent
//-----------------------------------------------------------------------------
bool eControl::event(QEvent* event)
{
	switch(event->type())
	{
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseMove:
		{
			QMouseEvent* me = (QMouseEvent*)event;
			bool ok = event->type() == QEvent::MouseMove ? ((me->buttons() & Qt::LeftButton) != 0) : (me->button() == Qt::LeftButton);
			if(ok)
			{
				float x = float(me->x())/width();
				float y = float(me->y())/height();
				OnTouch(x, y, event->type() != QEvent::MouseButtonRelease, 0);
				return true;
			}
		}
		return true;
	case QEvent::TouchBegin:
	case QEvent::TouchUpdate:
	case QEvent::TouchEnd:
		{
			QTouchEvent* te = (QTouchEvent*)event;
			foreach(const QTouchEvent::TouchPoint& p, te->touchPoints())
			{
				float x = p.pos().x()/width();
				float y = p.pos().x()/height();
				OnTouch(x, y, p.state() != Qt::TouchPointReleased, p.id());
			}
		}
		return true;
	default:
		return QWidget::event(event);
	}
}
//=============================================================================
//	eControl::paintEvent
//-----------------------------------------------------------------------------
void eControl::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(keyboard.rect(), Qt::black);
	painter.drawImage(QPointF(0, 0), keyboard_active ? keyboard : joystick);
}
//=============================================================================
//	eControl::keyPressEvent
//-----------------------------------------------------------------------------
void eControl::keyPressEvent(QKeyEvent* event)
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
//	eControl::keyReleaseEvent
//-----------------------------------------------------------------------------
void eControl::keyReleaseEvent(QKeyEvent* event)
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
//	eControl::EventKeyFlags
//-----------------------------------------------------------------------------
void eControl::EventKeyFlags(QKeyEvent* event, int* key, dword* flags) const
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
//	eControl::TranslateKey
//-----------------------------------------------------------------------------
void eControl::TranslateKey(int& key, dword& flags) const
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

#endif//USE_QT
