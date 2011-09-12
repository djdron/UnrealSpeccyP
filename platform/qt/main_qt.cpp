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

#include <QtGui/QApplication>
#include <QFile>
#ifdef Q_WS_S60
#include <QSystemScreenSaver>
#endif//Q_WS_S60
#include "qt_window.h"
#include "../../std_types.h"

byte sos128[16384];
byte sos48[16384];
byte service[16384];
byte dos513f[16384];

namespace xPlatform
{

static void InitResource(const char* name, byte* data)
{
	QFile f(name);
	f.open(QIODevice::ReadOnly);
	f.read((char*)data, f.size());
	f.close();
}

//=============================================================================
//	InitResources
//-----------------------------------------------------------------------------
static void InitResources()
{
	InitResource(":/rom/dos513f.rom", dos513f);
	InitResource(":/rom/service.rom", service);
	InitResource(":/rom/sos128.rom", sos128);
	InitResource(":/rom/sos48.rom", sos48);
}

}
//namespace xPlatform

//=============================================================================
//	main
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	using namespace xPlatform;
	QApplication a(argc, argv);
	InitResources();
	eWindow w;
	a.installEventFilter(&w);
#ifdef Q_WS_S60
	QTM_USE_NAMESPACE
	QSystemScreenSaver ss;
	ss.setScreenSaverInhibit();
	Qt::WindowFlags f = w.windowFlags();
	f |= Qt::WindowSoftkeysVisibleHint;
	f &= ~Qt::WindowSoftkeysRespondHint;
	w.setWindowFlags(f);
	w.showFullScreen();
#else//Q_WS_S60
	w.show();
#endif//Q_WS_S60
	return a.exec();
}

#endif//USE_QT
