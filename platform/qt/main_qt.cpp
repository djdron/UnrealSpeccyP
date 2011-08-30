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
#include "window.h"

byte sos128[16384];
byte sos48[16384];
byte service[16384];
byte dos513f[16384];
byte spxtrm4f[2048];

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
	InitResource(":/font/spxtrm4f.fnt", spxtrm4f);
}

//=============================================================================
//	main
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	InitResources();
	Window w;
#if defined(Q_WS_S60)
	w.showMaximized();
#else
	w.show();
#endif
	return a.exec();
}

#endif//USE_QT
