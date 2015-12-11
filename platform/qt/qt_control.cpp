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

#ifdef USE_QT

#include <QMouseEvent>
#include <QPainter>

#include "qt_control.h"
#include "../platform.h"
#include "../../tools/options.h"
#include "../../options_common.h"
#include "../touch_ui/tui_keyboard.h"
#include "../touch_ui/tui_joystick.h"

namespace xPlatform
{

static struct eOptionUseKeyboard : public xOptions::eOptionBool
{
	eOptionUseKeyboard() { Set(true); }
	virtual const char* Name() const { return "use keyboard"; }
	virtual int Order() const { return 5; }
} op_use_keyboard;

//=============================================================================
//	eControl::eControl
//-----------------------------------------------------------------------------
eControl::eControl(QWidget* parent) : QWidget(parent)
{
	keyboard.load(":/image/keyboard.png");
	joystick.load(":/image/joystick.png");
	setAttribute(Qt::WA_NoSystemBackground, true);
	setAttribute(Qt::WA_AcceptTouchEvents);
	setContextMenuPolicy(Qt::NoContextMenu);
}
//=============================================================================
//	eControl::ToggleKeyboard
//-----------------------------------------------------------------------------
void eControl::ToggleKeyboard()
{
	op_use_keyboard.Change();
	update();
}
//=============================================================================
//	eControl::OnTouch
//-----------------------------------------------------------------------------
void eControl::OnTouch(float x, float y, bool down, int pointer_id)
{
	if(op_use_keyboard)
		OnTouchKey(x, y, down, pointer_id);
	else
		OnTouchJoy(x, y, down, pointer_id);
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
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	QRect rect = painter.viewport();
	QSize size = keyboard.size();
	size.scale(rect.size(), Qt::KeepAspectRatio);
	painter.setViewport((rect.width() - size.width())/2, (rect.height() - size.height())/2, size.width(), size.height());
	painter.setWindow(keyboard.rect());
	painter.drawImage(0, 0, op_use_keyboard ? keyboard : joystick);
}
//=============================================================================
//	eControl::minimumSizeHint
//-----------------------------------------------------------------------------
QSize eControl::minimumSizeHint() const { return keyboard.size(); }

}
//namespace xPlatform

#endif//USE_QT
