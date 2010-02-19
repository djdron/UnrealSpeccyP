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
	, next(0), tshift(0), state(S_IDLE), state2(S_IDLE), cmd(0), data(0)
	, track(0), side(0), sector(0), rqs(0), status(0), stepdirection(0)
	, system(0), end_waiting_am(0), foundid(0), rwptr(0), rwlen(0), start_crc(0)
{
}
//=============================================================================
//	eWD1793::Init
//-----------------------------------------------------------------------------
void eWD1793::Init()
{
	fdd = &fdds[0];
}
//=============================================================================
//	eWD1793::Open
//-----------------------------------------------------------------------------
bool eWD1793::Open(const char* image, int fdd)
{
	assert(fdd >= 0 && fdd < 4);
	return fdds[fdd].Open(image);
}
//=============================================================================
//	eWD1793::Process
//-----------------------------------------------------------------------------
void eWD1793::Process(int tact)
{
	qword time = speccy->T() + tact;
	// inactive drives disregard HLT bit
	if(time > fdd->motor && (system & 0x08))
	{
		fdd->motor = 0;
	}
	fdd->DiskPresent() ? status &= ~ST_NOTRDY : status |= ST_NOTRDY;
	if(!(cmd & 0x80)) //seek/step commands
	{
		status &= ~(ST_TRK00|ST_INDEX);
		if(fdd->motor && (system & 0x08))
		{
			status |= ST_HEADL;
		}
		if(!fdd->cyl)
		{
			status |= ST_TRK00;
		}
		// todo: test spinning
		if(fdd->DiskPresent() && fdd->motor && ((time+tshift) % (Z80FQ/FDD_RPS) < (Z80FQ*4/1000)))
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
			state = state2;
			break;
		case S_DELAY_BEFORE_CMD:
			if(!wd93_nodelay && (cmd & CB_DELAY))
			{
				next += (Z80FQ*15/1000); // 15ms delay
			}
			status = (status|ST_BUSY) & ~(ST_DRQ|ST_LOST|ST_NOTFOUND|ST_RECORDT|ST_WRITEP);
			state = S_WAIT;
			state2 = S_CMD_RW;
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
				state2 = S_WRTRACK;
				break;
			}
			if((cmd & 0xf8) == 0xe0) //read track
			{
				Load();
				rwptr = 0;
				rwlen = fdd->Track().data_len;
				state2 = S_READ;
				GetIndex();
				break;
			}
			// else unknown command
			state = S_IDLE;
			break;
		case S_FOUND_NEXT_ID:
			if(!fdd->DiskPresent()) //no disk - wait again
			{
				end_waiting_am = next + 5*Z80FQ/FDD_RPS;
nextmk:			FindMarker();
				break;
			}
			if(next >= end_waiting_am || foundid == -1)
			{
				status |= ST_NOTFOUND;
				state = S_IDLE;
				break;
			}
			status &= ~ST_CRCERR;
			Load();
			if(!(cmd & 0x80)) //verify after seek
			{
				if(fdd->Sector(foundid).Cyl() != track)
					goto nextmk;
/*
				if(!fdd->Sector(foundid).c1)
				{
					status |= ST_CRCERR;
					goto nextmk;
				}
*/
				state = S_IDLE;
				break;
			}
			if((cmd & 0xf0) == 0xc0) //read AM
			{
				rwptr = fdd->Sector(foundid).id - fdd->Track().data;
				rwlen = 6;
read_first_byte:
				data = fdd->Track().data[rwptr++];
				rwlen--;
				rqs = R_DRQ;
				status |= ST_DRQ;
				next += fdd->TSByte();
				state = S_WAIT;
				state2 = S_READ;
				break;
			}
			// else R/W sector(s)
			if(fdd->Sector(foundid).Cyl() != track || fdd->Sector(foundid).Sec() != sector)
				goto nextmk;
			if((cmd & CB_SIDE_CMP_FLAG) && (((cmd >> CB_SIDE_SHIFT) ^ fdd->Sector(foundid).Side()) & 1))
				goto nextmk;
/*
			if(!fdd->Sector(foundid).c1)
			{
				status |= ST_CRCERR;
				goto nextmk;
			}
*/
			if(cmd & 0x20) //write sector(s)
			{
				rqs = R_DRQ;
				status |= ST_DRQ;
				next += fdd->TSByte() * 9;
				state = S_WAIT;
				state2 = S_WRSEC;
				break;
			}
			// read sector(s)
			if(!fdd->Sector(foundid).data)
				goto nextmk;
			fdd->Sector(foundid).data[-1] == 0xf8 ? status |= ST_RECORDT : status &= ~ST_RECORDT;
			rwptr = fdd->Sector(foundid).data - fdd->Track().data;
			rwlen = fdd->Sector(foundid).Len();
			goto read_first_byte;
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
				rwlen--;
				rqs = R_DRQ;
				status |= ST_DRQ;
				if(!wd93_nodelay)
				{
					next += fdd->TSByte();
				}
				state = S_WAIT;
				state2 = S_READ;
			}
			else
			{
				if((cmd & 0xe0) == 0x80) //read sector
				{
/*
					if(!ffdd->Sector(foundid).c2)
					{
						status |= ST_CRCERR;
					}
*/
					if(cmd & CB_MULTIPLE)
					{
						sector++;
						state = S_CMD_RW;
						break;
					}
				}
				if((cmd & 0xf0) == 0xc0) //read address
				{
/*
					if(!fdd->Sector(foundid).c1)
					{
						status |= ST_CRCERR;
					}
*/
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
			fdd->optype |= 1;
			rwptr = fdd->Sector(foundid).id + 6 + 11 + 11 - fdd->Track().data;
			for(rwlen = 0; rwlen < 12; rwlen++)
			{
				fdd->Write(rwptr++, 0);
			}
			for(rwlen = 0; rwlen < 3; rwlen++)
			{
				fdd->Write(rwptr++, 0xa1, true);
			}
			fdd->Write(rwptr++, (cmd & CB_WRITE_DEL) ? 0xf8 : 0xfb);
			rwlen = fdd->Sector(foundid).Len();
			state = S_WRITE;
			break;
		case S_WRITE:
			if(!Ready())
				break;
			if(rqs & R_DRQ)
			{
				status |= ST_LOST;
				data = 0;
				fdd->Write(rwptr++, data);
				rwlen--;
				if(rwptr == fdd->Track().data_len)
				{
					rwptr = 0;
				}
//				trkcache.sm = JUST_SEEK; // invalidate sectors
				if(rwlen)
				{
					if(!wd93_nodelay)
					{
						next += fdd->TSByte();
					}
					state = S_WAIT;
					state2 = S_WRITE;
					rqs = R_DRQ;
					status |= ST_DRQ;
				}
				else
				{
					int len = fdd->Sector(foundid).Len() + 1;
					byte sc[2056];
					if(rwptr < len)
					{
						memcpy(sc, fdd->Track().data + fdd->Track().data_len - rwptr, rwptr);
						memcpy(sc + rwptr, fdd->Track().data, len - rwptr);
					}
					else
					{
						memcpy(sc, fdd->Track().data + rwptr - len, len);
					}
					word crc = fdd->Crc(sc, len);
					fdd->Write(rwptr++, crc);
					fdd->Write(rwptr++, crc >> 8);
					fdd->Write(rwptr, 0xFF);
					if(cmd & CB_MULTIPLE)
					{
						sector++;
						state = S_CMD_RW;
						break;
					}
					state = S_IDLE;
				}
			}
			break;
		case S_WRTRACK:
			if(rqs & R_DRQ)
			{
				status |= ST_LOST;
				state = S_IDLE;
				break;
			}
			fdd->optype |= 2;
			state2 = S_WR_TRACK_DATA;
			start_crc = 0;
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
				fdd->Seek(track, side);
//				trkcache.sm = JUST_SEEK; // invalidate sectors
				if(!fdd->Track().data)
				{
					state = S_IDLE;
					break;
				}
				bool marker = false;
				byte v = data;
				word crc = 0;
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
					crc = fdd->Crc(fdd->Track().data + start_crc, rwptr - start_crc);
					v = crc & 0xff;
				}
				fdd->Write(rwptr++, v, marker);
				rwlen--;
				if(data == 0xf7)
				{
					fdd->Write(rwptr++, crc >> 8);
					rwlen--; // second byte of CRC16
				}
				if((int)rwlen > 0)
				{
					if(!wd93_nodelay)
					{
						next += fdd->TSByte();
					}
					state = S_WAIT;
					state2 = S_WR_TRACK_DATA;
					rqs = R_DRQ;
					status |= ST_DRQ;
					break;
				}
				state = S_IDLE;
				break;
			}
		case S_TYPE1_CMD:
			status = (status|ST_BUSY) & ~(ST_DRQ|ST_CRCERR|ST_SEEKERR|ST_WRITEP);
			rqs = (eRequest)0;
			if(fdd->WriteProtect())
			{
				status |= ST_WRITEP;
			}
			fdd->motor = next + 2 * Z80FQ;
			state2 = S_SEEKSTART; //default is seek/restore
			if(cmd & 0xE0) //single step
			{
				if(cmd & 0x40) stepdirection = (cmd & CB_SEEK_DIR) ? -1 : 1;
				state2 = S_STEP;
			}
			if(!wd93_nodelay)
			{
				next += 1 * Z80FQ / 1000;
			}
			state = S_WAIT;
			break;
		case S_STEP:
			{
				// TRK00 sampled only in RESTORE command
				if(!fdd->cyl && !(cmd & 0xf0))
				{
					track = 0;
					state = S_VERIFY;
					break;
				}
				if(!(cmd & 0xe0) || (cmd & CB_SEEK_TRKUPD))
				{
					track += stepdirection;
				}
				fdd->cyl += stepdirection;
				if(fdd->cyl == (byte)-1)
				{
					fdd->cyl = 0;
				}
				if(fdd->cyl >= MAX_PHYS_CYL)
				{
					fdd->cyl = MAX_PHYS_CYL;
				}
//				trkcache.Clear();
				static const dword steps[] = { 6, 12, 20, 30 };
				if(!wd93_nodelay)
				{
					next += steps[cmd & CB_SEEK_RATE] * Z80FQ / 1000;
				}
				state = S_WAIT;
				state2 = (cmd & 0xe0) ? S_VERIFY : S_SEEK;
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
			stepdirection = (data < track) ? -1 : 1;
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
			if(!fdd->cyl)
			{
				state = S_IDLE;
			}
			else
			{
				fdd->cyl--;
//				trkcache.Clear();
			}
			// if(!fdd->track) track = 0;
			next += 6 * Z80FQ / 1000;
			break;
		}
	}
}
//=============================================================================
//	eWD1793::FindMarker
//-----------------------------------------------------------------------------
void eWD1793::FindMarker()
{
	if(wd93_nodelay && fdd->cyl != track) fdd->cyl = track;
	Load();
	foundid = -1;
	dword wait = 10 * Z80FQ / FDD_RPS;
	if(fdd->motor && fdd->DiskPresent())
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
				foundid = i;
			}
		}
		wait = foundid != -1 ? wait * fdd->TSByte() : 10 * Z80FQ/FDD_RPS;
		if(wd93_nodelay && foundid != -1)
		{
			// adjust tshift, that id appares right under head
			dword pos = fdd->Sector(foundid).id - fdd->Track().data + 2;
			tshift = (dword)(((pos * fdd->TSByte()) - (next % div) + div) % div);
			wait = 100; // delay=0 causes fdc to search infinitely, when no matched id on track
		}
	} // else no index pulses - infinite wait, but now wait 10spins, and re-test if disk inserted
	next += wait;
	if(fdd->DiskPresent() && next > end_waiting_am)
	{
		next = end_waiting_am;
		foundid = -1;
	}
	state = S_WAIT;
	state2 = S_FOUND_NEXT_ID;
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
	state2 = state;
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
	rwlen = fdd->TSByte();
	state = S_WAIT;
}
//=============================================================================
//	eWD1793::Load
//-----------------------------------------------------------------------------
void eWD1793::Load()
{
	fdd->Seek(fdd->cyl, side);
}
//=============================================================================
//	eWD1793::IoRead
//-----------------------------------------------------------------------------
void eWD1793::IoRead(word port, byte* v, int tact)
{
	if(!rom->DosSelected() || (port & 0x1f) != 0x1f)
		return;
	Process(tact);
	*v = 0xff;
	byte p = port;
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
	if(!rom->DosSelected() || (port & 0x1F) != 0x1F)
		return;
	Process(tact);
	byte p = port;
	if(p == 0x1F) // cmd
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
		rqs = 0;
		if(cmd & 0x80) //read/write command
		{
			if(status & ST_NOTRDY) //abort if no disk
			{
				state = S_IDLE;
				rqs = R_INTRQ;
				return;
			}
			if(fdd->motor || wd93_nodelay) //continue disk spinning
			{
				fdd->motor = next + 2*Z80FQ;
			}
			state = S_DELAY_BEFORE_CMD;
			return;
		}
		state = S_TYPE1_CMD; //else seek/step command
	}
	else if(p == 0x3F) track = v;
	else if(p == 0x5F) sector = v;
	else if(p == 0x7F)
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
//		trkcache.Clear();
		if(!(v & 0x04)) //reset
		{
			status = ST_NOTRDY;
			rqs = R_INTRQ;
			fdd->motor = 0;
			state = S_IDLE;
		}
	}
}
