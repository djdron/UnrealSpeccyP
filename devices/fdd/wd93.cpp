#include "../../std.h"
#include "../../speccy.h"
#include "wd93.h"
#include "../memory.h"

const int Z80FQ = 3500000;		// todo: #define as (conf.frame*conf.intfq)
const int FDD_RPS = 5;			// rotation speed
const int MAX_PHYS_CYL = 86;	// don't seek over it

const bool wd93_nodelay = false;
bool trdos_wp[4] = { false, false, false, false };

//=============================================================================
//	eWD1793::OpenImage
//-----------------------------------------------------------------------------
bool eWD1793::OpenImage(int fdd_index, const char* name)
{
	assert(fdd_index >= 0 && fdd_index < 4);
	return fdd[fdd_index].OpenImage(name);
}
//=============================================================================
//	eWD1793::Process
//-----------------------------------------------------------------------------
void eWD1793::Process(int tact)
{
	time = speccy->T() + tact;
	// inactive drives disregard HLT bit
	if(time > seldrive->motor && (system & 0x08)) seldrive->motor = 0;
	if(seldrive->rawdata) status &= ~WDS_NOTRDY; else status |= WDS_NOTRDY;

	if(!(cmd & 0x80)) { // seek / step commands
		status &= ~(WDS_TRK00 | WDS_INDEX);
		if(seldrive->motor && (system & 0x08)) status |= WDS_HEADL;
		if(!seldrive->track) status |= WDS_TRK00;
		// todo: test spinning
		if(seldrive->rawdata && seldrive->motor && ((time+tshift) % (Z80FQ/FDD_RPS) < (Z80FQ*4/1000)))
			status |= WDS_INDEX; // index every turn, len=4ms (if disk present)
	}

	for (;;) {

		switch (state) {

			// ----------------------------------------------------

		 case S_IDLE:
			 status &= ~WDS_BUSY;
			 rqs = INTRQ;
			 return;

		 case S_WAIT:
			 if(time < next) return;
			 state = state2;
			 break;

			 // ----------------------------------------------------

		 case S_DELAY_BEFORE_CMD:
			 if(!wd93_nodelay && (cmd & CMD_DELAY)) next += (Z80FQ*15/1000); // 15ms delay
			 status = (status | WDS_BUSY) & ~(WDS_DRQ | WDS_LOST | WDS_NOTFOUND | WDS_RECORDT | WDS_WRITEP);
			 state2 = S_CMD_RW; state = S_WAIT;
			 break;

		 case S_CMD_RW:
			 if(((cmd & 0xE0) == 0xA0 || (cmd & 0xF0) == 0xF0) && trdos_wp[drive]) {
				 status |= WDS_WRITEP;
				 state = S_IDLE;
				 break;
			 }

			 if((cmd & 0xC0) == 0x80 || (cmd & 0xF8) == 0xC0) {
				 // read/write sectors or read am - find next AM
				 end_waiting_am = next + 5*Z80FQ/FDD_RPS; // max wait disk 5 turns
				 FindMarker();
				 break;
			 }

			 if((cmd & 0xF8) == 0xF0) { // write track
				 rqs = DRQ; status |= WDS_DRQ;
				 next += 3*trkcache.ts_byte;
				 state2 = S_WRTRACK; state = S_WAIT;
				 break;
			 }

			 if((cmd & 0xF8) == 0xE0) { // read track
				 Load(); rwptr = 0; rwlen = trkcache.trklen;
				 state2 = S_READ; GetIndex();
				 break;
			 }

			 // else unknown command
			 state = S_IDLE;
			 break;

		 case S_FOUND_NEXT_ID:
			 if(!seldrive->rawdata) { // no disk - wait again
				 end_waiting_am = next + 5*Z80FQ/FDD_RPS;
nextmk:
				 FindMarker();
				 break;
			 }
			 if(next >= end_waiting_am) { nf: status |= WDS_NOTFOUND; state = S_IDLE; break; }
			 if(foundid == -1) goto nf;

			 status &= ~WDS_CRCERR;
			 Load();

			 if(!(cmd & 0x80)) { // verify after seek
				 if(trkcache.hdr[foundid].c != track) goto nextmk;
				 if(!trkcache.hdr[foundid].c1) { status |= WDS_CRCERR; goto nextmk; }
				 state = S_IDLE; break;
			 }

			 if((cmd & 0xF0) == 0xC0) { // read AM
				 rwptr = trkcache.hdr[foundid].id - trkcache.trkd;
				 rwlen = 6;
read_first_byte:
				 data = trkcache.trkd[rwptr++]; rwlen--;
				 rqs = DRQ; status |= WDS_DRQ;
				 next += trkcache.ts_byte;
				 state = S_WAIT;
				 state2 = S_READ;
				 break;
			 }

			 // else R/W sector(s)
			 if(trkcache.hdr[foundid].c != track || trkcache.hdr[foundid].n != sector) goto nextmk;
			 if((cmd & CMD_SIDE_CMP_FLAG) && (((cmd >> CMD_SIDE_SHIFT) ^ trkcache.hdr[foundid].s) & 1)) goto nextmk;
			 if(!trkcache.hdr[foundid].c1) { status |= WDS_CRCERR; goto nextmk; }

			 if(cmd & 0x20) { // write sector(s)
				 rqs = DRQ; status |= WDS_DRQ;
				 next += trkcache.ts_byte*9;
				 state = S_WAIT; state2 = S_WRSEC;
				 break;
			 }

			 // read sector(s)
			 if(!trkcache.hdr[foundid].data) goto nextmk;
			 if(trkcache.hdr[foundid].data[-1] == 0xF8) status |= WDS_RECORDT; else status &= ~WDS_RECORDT;
			 rwptr = trkcache.hdr[foundid].data - trkcache.trkd;
			 rwlen = 128 << (trkcache.hdr[foundid].l & 3); // [vv]
			 goto read_first_byte;

		 case S_READ:
			 if(!Ready())
				 break;
			 Load();

			 if(rwlen) {
				 //				 trdos_load = ROMLED_TIME; // ???
				 if(rqs & DRQ) status |= WDS_LOST;
				 data = trkcache.trkd[rwptr++]; rwlen--;
				 rqs = DRQ; status |= WDS_DRQ;
				 if(!wd93_nodelay) next += trkcache.ts_byte;
				 state = S_WAIT;
				 state2 = S_READ;
			 } else {
				 if((cmd & 0xE0) == 0x80) { // read sector
					 if(!trkcache.hdr[foundid].c2) status |= WDS_CRCERR;
					 if(cmd & CMD_MULTIPLE) { sector++, state = S_CMD_RW; break; }
				 }
				 if((cmd & 0xF0) == 0xC0) // read address
					 if(!trkcache.hdr[foundid].c1) status |= WDS_CRCERR;
				 state = S_IDLE;
			 }
			 break;


		 case S_WRSEC:
			 Load();
			 if(rqs & DRQ) { status |= WDS_LOST; state = S_IDLE; break; }
			 seldrive->optype |= 1;
			 rwptr = trkcache.hdr[foundid].id + 6 + 11 + 11 - trkcache.trkd;
			 for (rwlen = 0; rwlen < 12; rwlen++) trkcache.Write(rwptr++, 0, 0);
			 for (rwlen = 0; rwlen < 3; rwlen++)  trkcache.Write(rwptr++, 0xA1, 1);
			 trkcache.Write(rwptr++, (cmd & CMD_WRITE_DEL)? 0xF8 : 0xFB, 0);
			 rwlen = 128 << (trkcache.hdr[foundid].l & 3); // [vv]
			 state = S_WRITE; break;

		 case S_WRITE:
			 if(!Ready())
				 break;
			 if(rqs & DRQ) status |= WDS_LOST, data = 0;
			 //			 trdos_save = ROMLED_TIME;
			 trkcache.Write(rwptr++, data, 0); rwlen--;
			 if(rwptr == trkcache.trklen) rwptr = 0;
			 trkcache.sm = JUST_SEEK; // invalidate sectors
			 if(rwlen) {
				 if(!wd93_nodelay) next += trkcache.ts_byte;
				 state = S_WAIT; state2 = S_WRITE;
				 rqs = DRQ; status |= WDS_DRQ;
			 } else {
				 int len = (128 << (trkcache.hdr[foundid].l & 3)) + 1; //[vv]
				 byte sc[2056];
				 if(rwptr < len)
					 memcpy(sc, trkcache.trkd + trkcache.trklen - rwptr, rwptr), memcpy(sc + rwptr, trkcache.trkd, len - rwptr);
				 else memcpy(sc, trkcache.trkd + rwptr - len, len);
				 dword crc = wd93_crc(sc, len);
				 trkcache.Write(rwptr++, crc, 0);
				 trkcache.Write(rwptr++, crc >> 8, 0);
				 trkcache.Write(rwptr, 0xFF, 0);
				 if(cmd & CMD_MULTIPLE) { sector++, state = S_CMD_RW; break; }
				 state = S_IDLE;
			 }
			 break;

		 case S_WRTRACK:
			 if(rqs & DRQ) { status |= WDS_LOST; state = S_IDLE; break; }
			 seldrive->optype |= 2;
			 state2 = S_WR_TRACK_DATA;
			 start_crc = 0;
			 GetIndex();
			 end_waiting_am = next + 5*Z80FQ/FDD_RPS;
			 break;

		 case S_WR_TRACK_DATA:
			 {
				 if(!Ready())
					 break;
				 //				 trdos_format = ROMLED_TIME;
				 if(rqs & DRQ) status |= WDS_LOST, data = 0;
				 trkcache.Seek(seldrive, seldrive->track, side, JUST_SEEK);
				 trkcache.sm = JUST_SEEK; // invalidate sectors

				 if(!trkcache.trkd)
				 {
					 state = S_IDLE;
					 break;
				 }

				 byte marker = 0, byte = data;
				 dword crc = 0;
				 if(data == 0xF5)
				 {
					 byte = 0xA1;
					 marker = 1;
					 start_crc = rwptr+1;
				 }
				 if(data == 0xF6)
				 {
					 byte = 0xC2;
					 marker = 1;
				 }
				 if(data == 0xF7)
				 {
					 crc = wd93_crc(trkcache.trkd+start_crc, rwptr-start_crc);
					 byte = crc & 0xFF;
				 }

				 trkcache.Write(rwptr++, byte, marker);
				 rwlen--;
				 if(data == 0xF7) trkcache.Write(rwptr++, crc >> 8, 0), rwlen--; // second byte of CRC16

				 if((int)rwlen > 0)
				 {
					 if(!wd93_nodelay) next += trkcache.ts_byte;
					 state2 = S_WR_TRACK_DATA; state = S_WAIT;
					 rqs = DRQ; status |= WDS_DRQ;
					 break;
				 }
				 state = S_IDLE;
				 break;
			 }

			 // ----------------------------------------------------

		 case S_TYPE1_CMD:
			 status = (status | WDS_BUSY) & ~(WDS_DRQ | WDS_CRCERR | WDS_SEEKERR | WDS_WRITEP);
			 rqs = 0;

			 if(trdos_wp[drive]) status |= WDS_WRITEP;
			 seldrive->motor = next + 2*Z80FQ;

			 state2 = S_SEEKSTART; // default is seek/restore
			 if(cmd & 0xE0) { // single step
				 if(cmd & 0x40) stepdirection = (cmd & CMD_SEEK_DIR) ? -1 : 1;
				 state2 = S_STEP;
			 }
			 if(!wd93_nodelay) next += 1*Z80FQ/1000;
			 state = S_WAIT; break;


		 case S_STEP:
			 {
//				 trdos_seek = ROMLED_TIME;

				 // TRK00 sampled only in RESTORE command
				 if(!seldrive->track && !(cmd & 0xF0)) { track = 0; state = S_VERIFY; break; }

				 if(!(cmd & 0xE0) || (cmd & CMD_SEEK_TRKUPD)) track += stepdirection;
				 seldrive->track += stepdirection;
				 if(seldrive->track == (byte)-1) seldrive->track = 0;
				 if(seldrive->track >= MAX_PHYS_CYL) seldrive->track = MAX_PHYS_CYL;
				 trkcache.Clear();

				 static const dword steps[] = { 6, 12, 20, 30 };
				 if(!wd93_nodelay) next += steps[cmd & CMD_SEEK_RATE]*Z80FQ/1000;
/*
				 #ifndef MOD_9X
				 if(!conf.wd93_nodelay && conf.fdd_noise) Beep((stepdirection > 0)? 600 : 800, 2);
				 #endif
*/
				 state2 = (cmd & 0xE0)? S_VERIFY : S_SEEK;
				 state = S_WAIT; break;
			 }

		 case S_SEEKSTART:
			 if(!(cmd & 0x10)) track = 0xFF, data = 0;
			 // state = S_SEEK; break;

		 case S_SEEK:
			 if(data == track) { state = S_VERIFY; break; }
			 stepdirection = (data < track) ? -1 : 1;
			 state = S_STEP; break;

		 case S_VERIFY:
			 if(!(cmd & CMD_SEEK_VERIFY)) { state = S_IDLE; break; }
			 end_waiting_am = next + 6*Z80FQ/FDD_RPS; // max wait disk 6 turns
			 Load(); FindMarker(); break;

		 case S_RESET: // seek to trk0, but don't be busy
			 if(!seldrive->track) state = S_IDLE;
			 else seldrive->track--, trkcache.Clear();
			 // if(!seldrive->track) track = 0;
			 next += 6*Z80FQ/1000;
			 break;

		 default:
			 assert(0); //errexit("WD1793 in wrong state");
		}
	}
}
//=============================================================================
//	eWD1793::FindMarker
//-----------------------------------------------------------------------------
void eWD1793::FindMarker()
{
	if(wd93_nodelay && seldrive->track != track) seldrive->track = track;
	Load();

	foundid = -1; dword wait = 10*Z80FQ/FDD_RPS;

	if(seldrive->motor && seldrive->rawdata) {
		dword div = trkcache.trklen*trkcache.ts_byte;
		dword i = (dword)((next+tshift) % div) / trkcache.ts_byte;
		wait = -1;
		for(int is = 0; is < trkcache.s; is++) {
			dword pos = trkcache.hdr[is].id - trkcache.trkd;
			dword dist = (pos > i)? pos-i : trkcache.trklen+pos-i;
			if(dist < wait) wait = dist, foundid = is;
		}

		if(foundid != -1) wait *= trkcache.ts_byte;
		else wait = 10*Z80FQ/FDD_RPS;

		if(wd93_nodelay && foundid != -1) {
			// adjust tshift, that id appares right under head
			dword pos = trkcache.hdr[foundid].id - trkcache.trkd + 2;
			tshift = (dword)(((pos * trkcache.ts_byte) - (next % div) + div) % div);
			wait = 100; // delay=0 causes fdc to search infinitely, when no matched id on track
		}

	} // else no index pulses - infinite wait, but now wait 10spins, and re-test if disk inserted

	next += wait;
	if(seldrive->rawdata && next > end_waiting_am) next = end_waiting_am, foundid = -1;
	state = S_WAIT; state2 = S_FOUND_NEXT_ID;
}
//=============================================================================
//	eWD1793::Ready
//-----------------------------------------------------------------------------
bool eWD1793::Ready()
{
	// fdc is too fast in no-delay mode, wait until cpu handles DRQ, but not more 'end_waiting_am'
	if(!wd93_nodelay || !(rqs & DRQ)) return true;
	if(next > end_waiting_am) return true;
	state2 = state; state = S_WAIT;
	next += trkcache.ts_byte;
	return false;
}
//=============================================================================
//	eWD1793::GetIndex
//-----------------------------------------------------------------------------
void eWD1793::GetIndex()
{
	dword trlen = trkcache.trklen*trkcache.ts_byte;
	dword ticks = (dword)((next+tshift) % trlen);
	if(!wd93_nodelay) next += (trlen - ticks);
	rwptr = 0; rwlen = trkcache.trklen; state = S_WAIT;
}
//=============================================================================
//	eWD1793::Load
//-----------------------------------------------------------------------------
void eWD1793::Load()
{
	trkcache.Seek(seldrive, seldrive->track, side, LOAD_SECTORS);
}
//=============================================================================
//	eWD1793::IoRead
//-----------------------------------------------------------------------------
void eWD1793::IoRead(word port, byte* v, int tact)
{
	if(!rom->DosSelected() || (port & 0x1F) != 0x1F)
		return;

	byte p = port;
	Process(tact);
	*v = 0xff;
	if(p & 0x80)		*v = rqs | 0x3F;
	else if(p == 0x1f)	{ rqs &= ~INTRQ; *v = status; }
	else if(p == 0x3f)	*v = track;
	else if(p == 0x5f)	*v = sector;
	else if(p == 0x7f)
	{
		status &= ~WDS_DRQ; rqs &= ~DRQ;
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

	byte p = port;
	Process(tact);
	if(p == 0x1F) { // cmd

		// force interrupt
		if((v & 0xF0) == 0xD0) {
			state = S_IDLE; rqs = INTRQ;
			status &= ~WDS_BUSY;
			return;
		}

		if(status & WDS_BUSY) return;
		cmd = v; next = speccy->T() + tact;
		status |= WDS_BUSY; rqs = 0;

		//-----------------------------------------------------------------------

		if(cmd & 0x80) // read/write command
		{
			// abort if no disk
			if(status & WDS_NOTRDY) {
				state = S_IDLE; rqs = INTRQ;
				return;
			}

			// continue disk spinning
			if(seldrive->motor || wd93_nodelay) seldrive->motor = next + 2*Z80FQ;

			state = S_DELAY_BEFORE_CMD;
			return;
		}

		// else seek/step command
		state = S_TYPE1_CMD;
		return;
	}

	//=======================================================================

	if(p == 0x3F) { track = v; return; }
	if(p == 0x5F) { sector = v; return; }
	if(p == 0x7F)
	{
		data = v, rqs &= ~DRQ, status &= ~WDS_DRQ; return;
	}

	if(p & 0x80) { // system
		system = v;
		drive = v & 3, side = 1 & ~(v >> 4);
		seldrive = fdd + drive;
		trkcache.Clear();
		if(!(v & 0x04)) { // reset
			status = WDS_NOTRDY;
			rqs = INTRQ;
			seldrive->motor = 0;
			state = S_IDLE;
#if 0 // move head to trk00
			steptime = 6 * (Z80FQ / 1000); // 6ms
			next += 1*Z80FQ/1000; // 1ms before command
			state = S_RESET;
			//seldrive->track = 0;
#endif
		}
	}
}
