/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../../std.h"
#include "../../speccy.h"
#include "wd1793.h"
#include "../memory.h"

const int Z80FQ = 3500000;		// todo: #define as (conf.frame*conf.intfq)
const int FDD_RPS = 5;			// rotation speed
const int MAX_PHYS_CYL = 86;	// don't seek over it

const bool wd93_nodelay = false;

//=============================================================================
//	eWD1793::eWD1793
//-----------------------------------------------------------------------------
eWD1793::eWD1793(eSpeccy* _speccy, eRom* _rom) : speccy(_speccy), rom(_rom)
	, next(0), tshift(0), state(S_IDLE), state_next(S_IDLE), cmd(0), data(0)
	, track(0), side(0), sector(0), direction(0), rqs(R_NONE), status(0)
	, system(0), end_waiting_am(0), found_sec(NULL), rwptr(0), rwlen(0), crc(0), start_crc(-1)
{
}
//=============================================================================
//	eWD1793::Init
//-----------------------------------------------------------------------------
void eWD1793::Init()
{
	fdd = fdds;
}
//=============================================================================
//	eWD1793::Open
//-----------------------------------------------------------------------------
bool eWD1793::Open(const char* type, int drive, const void* data, size_t data_size)
{
	assert(drive >= 0 && drive < FDD_COUNT);
	int current_fdd;
	for(current_fdd = FDD_COUNT; --current_fdd >= 0;)
	{
		if(fdd == &fdds[current_fdd])
			break;
	}
	if(drive == current_fdd)
	{
		found_sec = NULL;
		status = ST_NOTRDY;
		rqs = R_INTRQ;
		state = S_IDLE;
	}
	return fdds[drive].Open(type, data, data_size);
}
//=============================================================================
//	eWD1793::BootExist
//-----------------------------------------------------------------------------
bool eWD1793::BootExist(int drive)
{
	assert(drive >= 0 && drive < FDD_COUNT);
	return fdds[drive].BootExist();
}
//=============================================================================
//	eWD1793::Crc
//-----------------------------------------------------------------------------
const word crc_initial = 0xcdb4;
word eWD1793::Crc(byte* src, int size) const
{
	word crc = crc_initial;
	while(size--)
	{
		crc = Crc(*src++, crc);
	}
	return crc;
}
//=============================================================================
//	eWD1793::Crc
//-----------------------------------------------------------------------------
word eWD1793::Crc(byte v, word prev_crc = crc_initial) const
{
	dword crc = prev_crc ^ (v << 8);
	for(int i = 8; i; --i)
	{
		if((crc <<= 1) & 0x10000)
		{
			crc ^= 0x1021; // bit representation of x^12+x^5+1
		}
	}
	return crc;
}
//=============================================================================
//	eWD1793::Process
//-----------------------------------------------------------------------------
void eWD1793::Process(int tact)
{
	qword time = speccy->T() + tact;
	// inactive drives disregard HLT bit
	if(time > fdd->Motor() && (system & 0x08))
	{
		fdd->Motor(0);
	}
	fdd->DiskPresent() ? status &= ~ST_NOTRDY : status |= ST_NOTRDY;
	if(!(cmd & 0x80)) //seek/step commands
	{
		status &= ~(ST_TRK00|ST_INDEX);
		if(fdd->Motor() && (system & 0x08))
		{
			status |= ST_HEADL;
		}
		if(!fdd->Cyl())
		{
			status |= ST_TRK00;
		}
		// todo: test spinning
		if(fdd->DiskPresent() && fdd->Motor() && ((time+tshift) % (Z80FQ/FDD_RPS) < (Z80FQ*4/1000)))
		{
			status |= ST_INDEX; // index every turn, len=4ms (if disk present)
		}
	}
	for(;;)
	{
		switch (state)
		{
		case S_IDLE:
			status &= ~ST_BUSY;
			rqs = R_INTRQ;
			return;
		case S_WAIT:
			if(time < next)
				return;
			state = state_next;
			break;
		case S_DELAY_BEFORE_CMD:
			if(!wd93_nodelay && (cmd & CB_DELAY))
			{
				next += (Z80FQ*15/1000); // 15ms delay
			}
			status = (status|ST_BUSY) & ~(ST_DRQ|ST_LOST|ST_NOTFOUND|ST_RECORDT|ST_WRITEP);
			state = S_WAIT;
			state_next = S_CMD_RW;
			break;
		case S_CMD_RW:
			if(((cmd & 0xe0) == 0xa0 || (cmd & 0xf0) == 0xf0) && fdd->WriteProtect())
			{
				status |= ST_WRITEP;
				state = S_IDLE;
				break;
			}
			if((cmd & 0xc0) == 0x80 || (cmd & 0xf8) == 0xc0) //read/write sectors or read am - find next AM
			{
				end_waiting_am = next + 5*Z80FQ/FDD_RPS; // max wait disk 5 turns
				FindMarker();
				break;
			}
			if((cmd & 0xf8) == 0xf0) //write track
			{
				rqs = R_DRQ;
				status |= ST_DRQ;
				next += 3 * fdd->TSByte();
				state = S_WAIT;
				state_next = S_WRTRACK;
				break;
			}
			if((cmd & 0xf8) == 0xe0) //read track
			{
				Load();
				rwptr = 0;
				rwlen = fdd->Track().data_len;
				state_next = S_READ;
				GetIndex();
				break;
			}
			state = S_IDLE;
			break;
		case S_FOUND_NEXT_ID:
			if(!fdd->DiskPresent()) //no disk - wait again
			{
				end_waiting_am = next + 5*Z80FQ/FDD_RPS;
				FindMarker();
				break;
			}
			if(next >= end_waiting_am || !found_sec)
			{
				status |= ST_NOTFOUND;
				state = S_IDLE;
				break;
			}
			status &= ~ST_CRCERR;
			Load();
			if(!(cmd & 0x80)) //verify after seek
			{
				if(found_sec->Cyl() != track)
				{
					FindMarker();
					break;
				}
				if(Crc(found_sec->id - 1, 5) != found_sec->IdCrc())
				{
					status |= ST_CRCERR;
					FindMarker();
					break;
				}
				state = S_IDLE;
				break;
			}
			if((cmd & 0xf0) == 0xc0) //read AM
			{
				rwptr = found_sec->id - fdd->Track().data;
				rwlen = 6;
				ReadFirstByte();
				break;
			}
			// else R/W sector(s)
			if(found_sec->Cyl() != track || found_sec->Sec() != sector
				|| ((cmd&CB_SIDE_CMP) && (((cmd >> CB_SIDE_SHIFT) ^ found_sec->Side()) & 1)))
			{
				FindMarker();
				break;
			}
			if(Crc(found_sec->id - 1, 5) != found_sec->IdCrc())
			{
				status |= ST_CRCERR;
				FindMarker();
				break;
			}
			if(cmd & 0x20) //write sector(s)
			{
				rqs = R_DRQ;
				status |= ST_DRQ;
				next += fdd->TSByte() * 9;
				state = S_WAIT;
				state_next = S_WRSEC;
				break;
			}
			// read sector(s)
			if(!found_sec->data)
			{
				FindMarker();
				break;
			}
			found_sec->data[-1] == 0xf8 ? status |= ST_RECORDT : status &= ~ST_RECORDT;
			rwptr = found_sec->data - fdd->Track().data;
			rwlen = found_sec->Len();
			ReadFirstByte();
			break;
		case S_READ:
			if(!Ready())
				break;
			Load();
			if(rwlen)
			{
				if(rqs & R_DRQ)
				{
					status |= ST_LOST;
				}
				data = fdd->Track().data[rwptr++];
				crc = Crc(data, crc);
				rwlen--;
				rqs = R_DRQ;
				status |= ST_DRQ;
				if(!wd93_nodelay)
				{
					next += fdd->TSByte();
				}
				state = S_WAIT;
				state_next = S_READ;
			}
			else
			{
				if((cmd & 0xe0) == 0x80) //read sector
				{
					if(crc != found_sec->DataCrc())
					{
						status |= ST_CRCERR;
					}
					if(cmd & CB_MULTIPLE)
					{
						sector++;
						state = S_CMD_RW;
						break;
					}
				}
				if((cmd & 0xf0) == 0xc0) //read address
				{
					if(Crc(found_sec->id - 1, 5) != found_sec->IdCrc())
					{
						status |= ST_CRCERR;
					}
				}
				state = S_IDLE;
			}
			break;
		case S_WRSEC:
			Load();
			if(rqs & R_DRQ)
			{
				status |= ST_LOST;
				state = S_IDLE;
				break;
			}
			rwptr = found_sec->id + 6 + 22 - fdd->Track().data;
			for(rwlen = 0; rwlen < 12; rwlen++)
			{
				fdd->Write(rwptr++, 0);
			}
			for(rwlen = 0; rwlen < 3; rwlen++)
			{
				fdd->Write(rwptr++, 0xa1, true);
			}
			fdd->Write(rwptr++, (cmd & CB_WRITE_DEL) ? 0xf8 : 0xfb);
			crc = Crc(fdd->Track().data[rwptr - 1]);
			rwlen = found_sec->Len();
			state = S_WRITE;
			break;
		case S_WRITE:
			if(!Ready())
				break;
			if(rqs & R_DRQ)
			{
				status |= ST_LOST;
				data = 0;
			}
			fdd->Write(rwptr++, data);
			crc = Crc(data, crc);
			rwlen--;
			if(rwptr == fdd->Track().data_len)
			{
				rwptr = 0;
			}
			if(rwlen)
			{
				if(!wd93_nodelay)
				{
					next += fdd->TSByte();
				}
				state = S_WAIT;
				state_next = S_WRITE;
				rqs = R_DRQ;
				status |= ST_DRQ;
			}
			else
			{
				fdd->Write(rwptr++, crc >> 8);
				fdd->Write(rwptr++, (byte)crc);
				fdd->Write(rwptr, 0xff);
				if(cmd & CB_MULTIPLE)
				{
					sector++;
					state = S_CMD_RW;
					break;
				}
				state = S_IDLE;
			}
			break;
		case S_WRTRACK:
			if(rqs & R_DRQ)
			{
				status |= ST_LOST;
				state = S_IDLE;
				break;
			}
			state_next = S_WR_TRACK_DATA;
			start_crc = -1;
			GetIndex();
			end_waiting_am = next + 5 * Z80FQ/FDD_RPS;
			break;
		case S_WR_TRACK_DATA:
			{
				if(!Ready())
					break;
				if(rqs & R_DRQ)
				{
					status |= ST_LOST;
					data = 0;
				}
				Load();
				if(!fdd->Track().data)
				{
					state = S_IDLE;
					break;
				}
				bool marker = false;
				byte v = data;
				if(data == 0xf5)
				{
					v = 0xa1;
					marker = true;
					start_crc = rwptr + 1;
				}
				if(data == 0xf6)
				{
					v = 0xc2;
					marker = true;
				}
				if(data == 0xf7)
				{
					start_crc = -1;
					fdd->Write(rwptr++, crc >> 8);
					rwlen--;
					v = (byte)crc;
				}
				if(start_crc >= 0 && rwptr >= start_crc)
				{
					crc = rwptr == start_crc ? Crc(v) : Crc(v, crc);
				}
				fdd->Write(rwptr++, v, marker);
				rwlen--;
				if(rwlen > 0)
				{
					if(!wd93_nodelay)
					{
						next += fdd->TSByte();
					}
					state = S_WAIT;
					state_next = S_WR_TRACK_DATA;
					rqs = R_DRQ;
					status |= ST_DRQ;
					break;
				}
				fdd->Track().Update();
				state = S_IDLE;
				break;
			}
		case S_TYPE1_CMD:
			status = (status|ST_BUSY) & ~(ST_DRQ|ST_CRCERR|ST_SEEKERR|ST_WRITEP);
			rqs = R_NONE;
			if(fdd->WriteProtect())
			{
				status |= ST_WRITEP;
			}
			fdd->Motor(next + 2 * Z80FQ);
			state_next = S_SEEKSTART; //default is seek/restore
			if(cmd & 0xE0) //single step
			{
				if(cmd & 0x40) direction = (cmd & CB_SEEK_DIR) ? -1 : 1;
				state_next = S_STEP;
			}
			if(!wd93_nodelay)
			{
				next += 1 * Z80FQ / 1000;
			}
			state = S_WAIT;
			break;
		case S_STEP:
			{
				if(!fdd->Cyl() && !(cmd & 0xf0)) //TRK00 sampled only in RESTORE command
				{
					track = 0;
					state = S_VERIFY;
					break;
				}
				if(!(cmd & 0xe0) || (cmd & CB_SEEK_TRKUPD))
				{
					track += direction;
				}
				int cyl = fdd->Cyl() + direction;
				if(cyl < 0)
				{
					cyl = 0;
				}
				if(cyl >= MAX_PHYS_CYL)
				{
					cyl = MAX_PHYS_CYL;
				}
				fdd->Cyl(cyl);
				static const dword steps[] = { 6, 12, 20, 30 };
				if(!wd93_nodelay)
				{
					next += steps[cmd & CB_SEEK_RATE] * Z80FQ / 1000;
				}
				state = S_WAIT;
				state_next = (cmd & 0xe0) ? S_VERIFY : S_SEEK;
				break;
			}
		case S_SEEKSTART:
			if(!(cmd & 0x10))
			{
				track = 0xff;
				data = 0;
			}
		case S_SEEK:
			if(data == track)
			{
				state = S_VERIFY;
				break;
			}
			direction = (data < track) ? -1 : 1;
			state = S_STEP;
			break;
		case S_VERIFY:
			if(!(cmd & CB_SEEK_VERIFY))
			{
				state = S_IDLE;
				break;
			}
			end_waiting_am = next + 6 * Z80FQ / FDD_RPS; //max wait disk 6 turns
			Load();
			FindMarker();
			break;
		case S_RESET: //seek to trk0, but don't be busy
			if(!fdd->Cyl())
			{
				state = S_IDLE;
			}
			else
			{
				fdd->Cyl(fdd->Cyl() - 1);
			}
			next += 6 * Z80FQ / 1000;
			break;
		}
	}
}
//=============================================================================
//	eWD1793::ReadFirstByte
//-----------------------------------------------------------------------------
void eWD1793::ReadFirstByte()
{
	crc = Crc(fdd->Track().data[rwptr - 1]);
	data = fdd->Track().data[rwptr++];
	crc = Crc(data, crc);
	rwlen--;
	rqs = R_DRQ;
	status |= ST_DRQ;
	next += fdd->TSByte();
	state = S_WAIT;
	state_next = S_READ;
}
//=============================================================================
//	eWD1793::FindMarker
//-----------------------------------------------------------------------------
void eWD1793::FindMarker()
{
	if(wd93_nodelay && fdd->Cyl() != track)
	{
		fdd->Cyl(track);
	}
	Load();
	found_sec = NULL;
	dword wait = 10 * Z80FQ / FDD_RPS;
	if(fdd->Motor() && fdd->DiskPresent())
	{
		dword div = fdd->Track().data_len * fdd->TSByte();
		dword idx = (dword)((next + tshift) % div) / fdd->TSByte();
		wait = -1;
		for(int i = 0; i < fdd->Track().sectors_amount; ++i)
		{
			dword pos = fdd->Sector(i).id - fdd->Track().data;
			dword dist = (pos > idx) ? pos - idx : fdd->Track().data_len + pos - idx;
			if(dist < wait)
			{
				wait = dist;
				found_sec = &fdd->Sector(i);
			}
		}
		wait = found_sec ? wait * fdd->TSByte() : 10 * Z80FQ/FDD_RPS;
		if(wd93_nodelay && found_sec)
		{
			// adjust tshift, that id appares right under head
			dword pos = found_sec->id - fdd->Track().data + 2;
			tshift = (dword)(((pos * fdd->TSByte()) - (next % div) + div) % div);
			wait = 100; // delay=0 causes fdc to search infinitely, when no matched id on track
		}
	} // else no index pulses - infinite wait, but now wait 10spins, and re-test if disk inserted
	next += wait;
	if(fdd->DiskPresent() && next > end_waiting_am)
	{
		next = end_waiting_am;
		found_sec = NULL;
	}
	state = S_WAIT;
	state_next = S_FOUND_NEXT_ID;
}
//=============================================================================
//	eWD1793::Ready
//-----------------------------------------------------------------------------
bool eWD1793::Ready()
{
	// fdc is too fast in no-delay mode, wait until cpu handles DRQ, but not more 'end_waiting_am'
	if(!wd93_nodelay || !(rqs & R_DRQ))
		return true;
	if(next > end_waiting_am)
		return true;
	state = S_WAIT;
	state_next = state;
	next += fdd->TSByte();
	return false;
}
//=============================================================================
//	eWD1793::GetIndex
//-----------------------------------------------------------------------------
void eWD1793::GetIndex()
{
	dword trlen = fdd->Track().data_len * fdd->TSByte();
	dword ticks = (dword)((next + tshift) % trlen);
	if(!wd93_nodelay)
	{
		next += (trlen - ticks);
	}
	rwptr = 0;
	rwlen = fdd->Track().data_len;
	state = S_WAIT;
}
//=============================================================================
//	eWD1793::Load
//-----------------------------------------------------------------------------
void eWD1793::Load()
{
	fdd->Seek(fdd->Cyl(), side);
}
//=============================================================================
//	eWD1793::IoRead
//-----------------------------------------------------------------------------
bool eWD1793::IoRead(word port) const
{
	if((port&0x1f) != 0x1f)
		return false;
	byte p = (byte)port;
	return p == 0x1f || p == 0x3f || p == 0x5f || p == 0x7f || p & 0x80;
}
//=============================================================================
//	eWD1793::IoWrite
//-----------------------------------------------------------------------------
bool eWD1793::IoWrite(word port) const
{
	if((port&0x1f) != 0x1f)
		return false;
	byte p = (byte)port;
	return p == 0x1f || p == 0x3f || p == 0x5f || p == 0x7f || p & 0x80;
}
//=============================================================================
//	eWD1793::IoRead
//-----------------------------------------------------------------------------
void eWD1793::IoRead(word port, byte* v, int tact)
{
	if(!rom->DosSelected())
		return;
	Process(tact);
	*v = 0xff;
	byte p = (byte)port;
	if(p & 0x80)		*v = rqs | 0x3F;
	else if(p == 0x1f)
	{
		rqs &= ~R_INTRQ;
		*v = status;
	}
	else if(p == 0x3f)	*v = track;
	else if(p == 0x5f)	*v = sector;
	else if(p == 0x7f)
	{
		status &= ~ST_DRQ;
		rqs &= ~R_DRQ;
		*v = data;
	}
}
//=============================================================================
//	eWD1793::IoWrite
//-----------------------------------------------------------------------------
void eWD1793::IoWrite(word port, byte v, int tact)
{
	if(!rom->DosSelected())
		return;
	Process(tact);
	byte p = (byte)port;
	if(p == 0x1f) // cmd
	{
		if((v & 0xf0) == 0xd0) // force interrupt
		{
			state = S_IDLE;
			rqs = R_INTRQ;
			status &= ~ST_BUSY;
			return;
		}
		if(status & ST_BUSY)
			return;
		cmd = v;
		next = speccy->T() + tact;
		status |= ST_BUSY;
		rqs = R_NONE;
		if(cmd & 0x80) //read/write command
		{
			if(status & ST_NOTRDY) //abort if no disk
			{
				state = S_IDLE;
				rqs = R_INTRQ;
				return;
			}
			if(fdd->Motor() || wd93_nodelay) //continue disk spinning
			{
				fdd->Motor(next + 2*Z80FQ);
			}
			state = S_DELAY_BEFORE_CMD;
			return;
		}
		state = S_TYPE1_CMD; //else seek/step command
	}
	else if(p == 0x3f) track = v;
	else if(p == 0x5f) sector = v;
	else if(p == 0x7f)
	{
		data = v;
		rqs &= ~R_DRQ;
		status &= ~ST_DRQ;
	}
	else if(p & 0x80) //system
	{
		system = v;
		fdd = &fdds[v & 3];
		side = 1 & ~(v >> 4);
		if(!(v & 0x04)) //reset
		{
			status = ST_NOTRDY;
			rqs = R_INTRQ;
			fdd->Motor(0);
			state = S_IDLE;
		}
	}
}
