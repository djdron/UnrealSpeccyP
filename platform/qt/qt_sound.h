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

#ifndef QT_SOUND_H
#define QT_SOUND_H

#include <QIODevice>
#include "../../std_types.h"

#pragma once

class eAudioStream : public QIODevice
{
	Q_OBJECT
public:
	eAudioStream(QObject* parent) : QIODevice(parent), ready(0) { open(QIODevice::ReadOnly); }
	virtual qint64 readData(char* data, qint64 maxlen);
	virtual qint64 writeData(const char *data, qint64 len) { return 0; }

	void	Fill(void* data, size_t size);
	void	Use(void* out, size_t size);
	qint64	Ready() const { return ready; }

protected:
	enum { BUF_SIZE = 65536 };
	byte	buffer[BUF_SIZE];
	size_t	ready;
};

#endif // QT_SOUND_H
