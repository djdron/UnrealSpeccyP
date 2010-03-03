#include "../../std.h"
#include "../../speccy.h"
#include "../../z80/z80.h"
#include "tape.h"

void eTape::Init()
{
	eInherited::Init();
	max_pulses = 0;
	tape_err = 0;

	tape_image = NULL;
	tape_imagesize = 0;

	tapeinfo = NULL;
	tape_infosize = 0;

	appendable = 0;
}

void eTape::Reset()
{
	eInherited::Reset();
	ResetTape();
}
void eTape::Start()
{
	StartTape();
}
void eTape::Stop()
{
	StopTape();
}

void eTape::IoRead(word port, byte* v, int tact)
{
	if(port & 1)
		return;
	*v |= TapeBit(tact) & 0x40;
}

// tape image contains indexes in tape_pulse[]

dword eTape::FindPulse(dword t)
{
	if(max_pulses < 0x100)
	{
		for(dword i = 0; i < max_pulses; i++)
			if(tape_pulse[i] == t)
				return i;
		tape_pulse[max_pulses] = t;
		return max_pulses++;
	}
	tape_err = 1;
	dword nearest = 0;
	int delta = 0x7FFFFFFF;
	for(dword i = 0; i < 0x100; i++)
		if(delta > abs((int)t - (int)tape_pulse[i]))
			nearest = i, delta = abs((int)t - (int)tape_pulse[i]);
	return nearest;
}

void eTape::FindTapeIndex()
{
	for(dword i = 0; i < tape_infosize; i++)
		if(tape.play_pointer >= tape_image + tapeinfo[i].pos)
			tape.index = i;
}

void eTape::FindTapeSizes()
{
	for(dword i = 0; i < tape_infosize; i++)
	{
		dword end = (i == tape_infosize - 1) ? tape_imagesize
				: tapeinfo[i + 1].pos;
		dword sz = 0;
		for(dword j = tapeinfo[i].pos; j < end; j++)
			sz += tape_pulse[tape_image[j]];
		tapeinfo[i].t_size = sz;
	}
}

void eTape::StopTape()
{
	FindTapeIndex();
	if(tape.play_pointer == tape.end_of_tape)
		tape.index = 0;
	tape.play_pointer = 0;
	tape.edge_change = 0x7FFFFFFFFFFFFFFF;
	tape.tape_bit = -1;
	speccy->CPU()->FastEmul(NULL);
}

void eTape::ResetTape()
{
	tape.index = 0;
	tape.play_pointer = 0;
	tape.edge_change = 0x7FFFFFFFFFFFFFFF;
	tape.tape_bit = -1;
	speccy->CPU()->FastEmul(NULL);
}

void eTape::StartTape()
{
	if(!tape_image)
		return;
	tape.play_pointer = tape_image + tapeinfo[tape.index].pos;
	tape.end_of_tape = tape_image + tape_imagesize;
	tape.edge_change = speccy->T();
	tape.tape_bit = -1;
//	speccy->CPU()->FastEmul(FastTapeEmul);
}

void eTape::CloseTape()
{
	speccy->CPU()->FastEmul(NULL);
	if(tape_image)
	{
		free(tape_image);
		tape_image = 0;
	}
	if(tapeinfo)
	{
		free(tapeinfo);
		tapeinfo = 0;
	}
	tape.play_pointer = 0; // stop tape
	tape.index = 0; // rewind tape
	tape_err = max_pulses = tape_imagesize = tape_infosize = 0;
	tape.edge_change = 0x7FFFFFFFFFFFFFFF;
	tape.tape_bit = -1;
}

#define align_by(a,b) (((dword)(a) + ((b)-1)) & ~((b)-1))

void eTape::Reserve(dword datasize)
{
	const int blocksize = 16384;
	dword newsize = align_by(datasize+tape_imagesize+1, blocksize);
	if(!tape_image)
		tape_image = (byte*)malloc(newsize);
	if(align_by(tape_imagesize, blocksize) < newsize)
		tape_image = (byte*)realloc(tape_image, newsize);
}

void eTape::MakeBlock(byte*data, dword size, dword pilot_t, dword s1_t,
		dword s2_t, dword zero_t, dword one_t, dword pilot_len, dword pause,
		byte last)
{
	Reserve(size * 16 + pilot_len + 3);
	if(pilot_len != (dword)-1)
	{
		dword t = FindPulse(pilot_t);
		for(dword i = 0; i < pilot_len; i++)
			tape_image[tape_imagesize++] = t;
		tape_image[tape_imagesize++] = FindPulse(s1_t);
		tape_image[tape_imagesize++] = FindPulse(s2_t);
	}
	dword t0 = FindPulse(zero_t), t1 = FindPulse(one_t);
	for(; size > 1; size--, data++)
		for(byte j = 0x80; j; j >>= 1)
			tape_image[tape_imagesize++] = (*data & j) ? t1 : t0, tape_image[tape_imagesize++]
					= (*data & j) ? t1 : t0;
	for(byte j = 0x80; j != (byte)(0x80 >> last); j >>= 1) // last byte
		tape_image[tape_imagesize++] = (*data & j) ? t1 : t0, tape_image[tape_imagesize++]
				= (*data & j) ? t1 : t0;
	if(pause)
		tape_image[tape_imagesize++] = FindPulse(pause * 3500);
}

void eTape::Desc(byte*data, dword size, char *dst)
{
	byte crc = 0;
	char prg[10];
	dword i; //Alone Coder 0.36.7
	for(/*dword*/i = 0; i < size; i++)
		crc ^= data[i];
	if(!*data && size == 19 && (data[1] == 0 || data[1] == 3))
	{
		for(i = 0; i < 10; i++)
			prg[i] = (data[i + 2] < ' ' || data[i + 2] >= 0x80) ? '?' : data[i
					+ 2];
		for(i = 9; i && prg[i] == ' '; prg[i--] = 0)
			;
		sprintf(dst, "%s: \"%s\" %d,%d", data[1] ? "Bytes" : "Program", prg,
				*(word*)(data + 14), *(word*)(data + 12));
	}
	else if(*data == 0xFF)
		sprintf(dst, "data block, %d bytes", size - 2);
	else
		sprintf(dst, "#%02X block, %d bytes", *data, size - 2);
	sprintf(dst + strlen(dst), ", crc %s", crc ? "bad" : "ok");
}

void eTape::AllocInfocell()
{
	tapeinfo = (TAPEINFO*)realloc(tapeinfo, (tape_infosize + 1)
			* sizeof(TAPEINFO));
	tapeinfo[tape_infosize].pos = tape_imagesize;
	appendable = 0;
}

void eTape::NamedCell(const void *nm, dword sz)
{
	AllocInfocell();
	if(sz)
		memcpy(tapeinfo[tape_infosize].desc, nm, sz), tapeinfo[tape_infosize].desc[sz]
				= 0;
	else
		strcpy(tapeinfo[tape_infosize].desc, (const char*)nm);
	tape_infosize++;
}

bool eTape::Open(const char* file)
{
	int l = strlen(file);
	if(l < 4)
		return false;
	FILE* f = fopen(file, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	byte* buf = new byte[size];
	size_t r = fread(buf, 1, size, f);
	fclose(f);
	if(r != size)
	{
		delete buf;
		return false;
	}
	bool ok = false;
	const char* ext = file + l - 4;
	if(!strcmp(ext, ".tap") || !strcmp(ext, ".TAP"))
		ok = ParseTAP(buf, size);
	else if(!strcmp(ext, ".csw") || !strcmp(ext, ".CSW"))
		ok = ParseCSW(buf, size);
	else if(!strcmp(ext, ".tzx") || !strcmp(ext, ".TZX"))
		ok = ParseTZX(buf, size);
	delete buf;
	return ok;
}

bool eTape::ParseTAP(byte* buf, size_t buf_size)
{
	byte* ptr = buf;
	CloseTape();
	while(ptr < buf + buf_size)
	{
		dword size = *(word*)ptr;
		ptr += 2;
		if(!size)
			break;
		AllocInfocell();
		Desc(ptr, size, tapeinfo[tape_infosize].desc);
		tape_infosize++;
		MakeBlock(ptr, size, 2168, 667, 735, 855, 1710, (*ptr < 4) ? 8064
				: 3220, 1000);
		ptr += size;
	}
	FindTapeSizes();
	return (ptr == buf + buf_size);
}

bool eTape::ParseCSW(byte* buf, size_t buf_size)
{
	const dword Z80FQ = 3500000;
	CloseTape();
	NamedCell("CSW tape image");
	if(buf[0x1B] != 1)
		return false; // unknown compression type
	dword rate = Z80FQ / *(word*)(buf + 0x19); // usually 3.5mhz / 44khz
	if(!rate)
		return false;
	Reserve(buf_size - 0x18);
	if(!(buf[0x1C] & 1))
		tape_image[tape_imagesize++] = FindPulse(1);
	for(byte* ptr = buf + 0x20; ptr < buf + buf_size;)
	{
		dword len = *ptr++ * rate;
		if(!len)
		{
			len = *(dword*)ptr / rate;
			ptr += 4;
		}
		tape_image[tape_imagesize++] = FindPulse(len);
	}
	tape_image[tape_imagesize++] = FindPulse(Z80FQ / 10);
	FindTapeSizes();
	return true;
}

void eTape::CreateAppendableBlock()
{
	if(!tape_infosize || appendable)
		return;
	NamedCell("set of pulses");
	appendable = 1;
}

void eTape::ParseHardware(byte* ptr)
{
	dword n = *ptr++;
	if(!n)
		return;
	NamedCell("- HARDWARE TYPE ");
	static char ids[] = "computer\0"
		"ZX Spectrum 16k\0"
		"ZX Spectrum 48k, Plus\0"
		"ZX Spectrum 48k ISSUE 1\0"
		"ZX Spectrum 128k (Sinclair)\0"
		"ZX Spectrum 128k +2 (Grey case)\0"
		"ZX Spectrum 128k +2A, +3\0"
		"Timex Sinclair TC-2048\0"
		"Timex Sinclair TS-2068\0"
		"Pentagon 128\0"
		"Sam Coupe\0"
		"Didaktik M\0"
		"Didaktik Gama\0"
		"ZX-81 or TS-1000 with  1k RAM\0"
		"ZX-81 or TS-1000 with 16k RAM or more\0"
		"ZX Spectrum 128k, Spanish version\0"
		"ZX Spectrum, Arabic version\0"
		"TK 90-X\0"
		"TK 95\0"
		"Byte\0"
		"Elwro\0"
		"ZS Scorpion\0"
		"Amstrad CPC 464\0"
		"Amstrad CPC 664\0"
		"Amstrad CPC 6128\0"
		"Amstrad CPC 464+\0"
		"Amstrad CPC 6128+\0"
		"Jupiter ACE\0"
		"Enterprise\0"
		"Commodore 64\0"
		"Commodore 128\0"
		"\0"
		"ext. storage\0"
		"Microdrive\0"
		"Opus Discovery\0"
		"Disciple\0"
		"Plus-D\0"
		"Rotronics Wafadrive\0"
		"TR-DOS (BetaDisk)\0"
		"Byte Drive\0"
		"Watsford\0"
		"FIZ\0"
		"Radofin\0"
		"Didaktik disk drives\0"
		"BS-DOS (MB-02)\0"
		"ZX Spectrum +3 disk drive\0"
		"JLO (Oliger) disk interface\0"
		"FDD3000\0"
		"Zebra disk drive\0"
		"Ramex Millenia\0"
		"Larken\0"
		"\0"
		"ROM/RAM type add-on\0"
		"Sam Ram\0"
		"Multiface\0"
		"Multiface 128k\0"
		"Multiface +3\0"
		"MultiPrint\0"
		"MB-02 ROM/RAM expansion\0"
		"\0"
		"sound device\0"
		"Classic AY hardware\0"
		"Fuller Box AY sound hardware\0"
		"Currah microSpeech\0"
		"SpecDrum\0"
		"AY ACB stereo; Melodik\0"
		"AY ABC stereo\0"
		"\0"
		"joystick\0"
		"Kempston\0"
		"Cursor, Protek, AGF\0"
		"Sinclair 2\0"
		"Sinclair 1\0"
		"Fuller\0"
		"\0"
		"mice\0"
		"AMX mouse\0"
		"Kempston mouse\0"
		"\0"
		"other controller\0"
		"Trickstick\0"
		"ZX Light Gun\0"
		"Zebra Graphics Tablet\0"
		"\0"
		"serial port\0"
		"ZX Interface 1\0"
		"ZX Spectrum 128k\0"
		"\0"
		"parallel port\0"
		"Kempston S\0"
		"Kempston E\0"
		"ZX Spectrum 128k +2A, +3\0"
		"Tasman\0"
		"DK'Tronics\0"
		"Hilderbay\0"
		"INES Printerface\0"
		"ZX LPrint Interface 3\0"
		"MultiPrint\0"
		"Opus Discovery\0"
		"Standard 8255 chip with ports 31,63,95\0"
		"\0"
		"printer\0"
		"ZX Printer, Alphacom 32 & compatibles\0"
		"Generic Printer\0"
		"EPSON Compatible\0"
		"\0"
		"modem\0"
		"VTX 5000\0"
		"T/S 2050 or Westridge 2050\0"
		"\0"
		"digitaiser\0"
		"RD Digital Tracer\0"
		"DK'Tronics Light Pen\0"
		"British MicroGraph Pad\0"
		"\0"
		"network adapter\0"
		"ZX Interface 1\0"
		"\0"
		"keyboard / keypad\0"
		"Keypad for ZX Spectrum 128k\0"
		"\0"
		"AD/DA converter\0"
		"Harley Systems ADC 8.2\0"
		"Blackboard Electronics\0"
		"\0"
		"EPROM Programmer\0"
		"Orme Electronics\0"
		"\0"
		"\0";
	for(dword i = 0; i < n; i++)
	{
		byte type_n = *ptr++;
		byte id_n = *ptr++;
		byte value_n = *ptr++;
		const char* type = ids, *id, *value;
		dword j; //Alone Coder 0.36.7
		for(/*dword*/j = 0; j < type_n; j++)
		{
			if(!*type)
				break;
			while(*(short*)type)
				type++;
			type += 2;
		}
		if(!*type)
			type = id = "??";
		else
		{
			id = type + strlen(type) + 1;
			for(j = 0; j < id_n; j++)
			{
				if(!*id)
				{
					id = "??";
					break;
				}
				id += strlen(id) + 1;
			}
		}
		switch(value_n)
		{
		case 0:
			value = "compatible with";
			break;
		case 1:
			value = "uses";
			break;
		case 2:
			value = "compatible, but doesn't use";
			break;
		case 3:
			value = "incompatible with";
			break;
		default:
			value = "??";
		}
		char bf[512];
		sprintf(bf, "%s %s: %s", value, type, id);
		NamedCell(bf);
	}
	NamedCell("-");
}

bool eTape::ParseTZX(byte* buf, size_t buf_size)
{
	byte* ptr = buf;
	CloseTape();
	dword size, pause, i, j, n, t, t0;
	byte pl, last, *end;
	char *p;
	dword loop_n = 0, loop_p = 0;
	char nm[512];
	while(ptr < buf + buf_size)
	{
		switch(*ptr++)
		{
		case 0x10: // normal block
			AllocInfocell();
			size = *(word*)(ptr + 2);
			pause = *(word*)ptr;
			ptr += 4;
			Desc(ptr, size, tapeinfo[tape_infosize].desc);
			tape_infosize++;
			MakeBlock(ptr, size, 2168, 667, 735, 855, 1710, (*ptr < 4) ? 8064
					: 3220, pause);
			ptr += size;
			break;
		case 0x11: // turbo block
			AllocInfocell();
			size = 0xFFFFFF & *(dword*)(ptr + 0x0F);
			Desc(ptr + 0x12, size, tapeinfo[tape_infosize].desc);
			tape_infosize++;
			MakeBlock(ptr + 0x12, size, *(word*)ptr, *(word*)(ptr + 2),
					*(word*)(ptr + 4), *(word*)(ptr + 6), *(word*)(ptr + 8),
					*(word*)(ptr + 10), *(word*)(ptr + 13), ptr[12]);
			// todo: test used bits - ptr+12
			ptr += size + 0x12;
			break;
		case 0x12: // pure tone
			CreateAppendableBlock();
			pl = FindPulse(*(word*)ptr);
			n = *(word*)(ptr + 2);
			Reserve(n);
			for(i = 0; i < n; i++)
				tape_image[tape_imagesize++] = pl;
			ptr += 4;
			break;
		case 0x13: // sequence of pulses of different lengths
			CreateAppendableBlock();
			n = *ptr++;
			Reserve(n);
			for(i = 0; i < n; i++, ptr += 2)
				tape_image[tape_imagesize++] = FindPulse(*(word*)ptr);
			break;
		case 0x14: // pure data block
			CreateAppendableBlock();
			size = 0xFFFFFF & *(dword*)(ptr + 7);
			MakeBlock(ptr + 0x0A, size, 0, 0, 0, *(word*)ptr,
					*(word*)(ptr + 2), -1, *(word*)(ptr + 5), ptr[4]);
			ptr += size + 0x0A;
			break;
		case 0x15: // direct recording
			size = 0xFFFFFF & *(dword*)(ptr + 5);
			t0 = *(word*)ptr;
			pause = *(word*)(ptr + 2);
			last = ptr[4];
			NamedCell("direct recording");
			ptr += 8;
			pl = 0;
			n = 0;
			for(i = 0; i < size; i++) // count number of pulses
				for(j = 0x80; j; j >>= 1)
					if((ptr[i] ^ pl) & j)
						n++, pl ^= -1;
			t = 0;
			pl = 0;
			Reserve(n + 2);
			for(i = 1; i < size; i++, ptr++) // find pulses
				for(j = 0x80; j; j >>= 1)
				{
					t += t0;
					if((*ptr ^ pl) & j)
					{
						tape_image[tape_imagesize++] = FindPulse(t);
						pl ^= -1;
						t = 0;
					}
				}
			// find pulses - last byte
			for(j = 0x80; j != (byte)(0x80 >> last); j >>= 1)
			{
				t += t0;
				if((*ptr ^ pl) & j)
				{
					tape_image[tape_imagesize++] = FindPulse(t);
					pl ^= -1;
					t = 0;
				}
			}
			ptr++;
			tape_image[tape_imagesize++] = FindPulse(t); // last pulse ???
			if(pause)
				tape_image[tape_imagesize++] = FindPulse(pause * 3500);
			break;
		case 0x20: // pause (silence) or 'stop the tape' command
			pause = *(word*)ptr;
			sprintf(nm, pause ? "pause %d ms" : "stop the tape", pause);
			NamedCell(nm);
			Reserve(2);
			ptr += 2;
			if(!pause)
			{ // at least 1ms pulse as specified in TZX 1.13
				tape_image[tape_imagesize++] = FindPulse(3500);
				pause = -1;
			}
			else
				pause *= 3500;
			tape_image[tape_imagesize++] = FindPulse(pause);
			break;
		case 0x21: // group start
			n = *ptr++;
			NamedCell(ptr, n);
			ptr += n;
			appendable = 1;
			break;
		case 0x22: // group end
			break;
		case 0x23: // jump to block
			NamedCell("* jump");
			ptr += 2;
			break;
		case 0x24: // loop start
			loop_n = *(word*)ptr;
			loop_p = tape_imagesize;
			ptr += 2;
			break;
		case 0x25: // loop end
			if(!loop_n)
				break;
			size = tape_imagesize - loop_p;
			Reserve((loop_n - 1) * size);
			for(i = 1; i < loop_n; i++)
				memcpy(tape_image + loop_p + i * size, tape_image + loop_p,
						size);
			tape_imagesize += (loop_n - 1) * size;
			loop_n = 0;
			break;
		case 0x26: // call
			NamedCell("* call");
			ptr += 2 + 2 * *(word*)ptr;
			break;
		case 0x27: // ret
			NamedCell("* return");
			break;
		case 0x28: // select block
			sprintf(nm, "* choice: ");
			n = ptr[2];
			p = (char*)ptr + 3;
			for(i = 0; i < n; i++)
			{
				if(i)
					strcat(nm, " / ");
				char *q = nm + strlen(nm);
				size = *(byte*)(p + 2);
				memcpy(q, p + 3, size);
				q[size] = 0;
				p += size + 3;
			}
			NamedCell(nm);
			ptr += 2 + *(word*)ptr;
			break;
		case 0x2A: // stop if 48k
			NamedCell("* stop if 48K");
			ptr += 4 + *(dword*)ptr;
			break;
		case 0x30: // text description
			n = *ptr++;
			NamedCell(ptr, n);
			ptr += n;
			appendable = 1;
			break;
		case 0x31: // message block
			NamedCell("- MESSAGE BLOCK ");
			end = ptr + 2 + ptr[1];
			pl = *end;
			*end = 0;
			for(p = (char*)ptr + 2; p < (char*)end; p++)
				if(*p == 0x0D)
					*p = 0;
			for(p = (char*)ptr + 2; p < (char*)end; p += strlen(p) + 1)
				NamedCell(p);
			*end = pl;
			ptr = end;
			NamedCell("-");
			break;
		case 0x32: // archive info
			NamedCell("- ARCHIVE INFO ");
			p = (char*)ptr + 3;
			for(i = 0; i < ptr[2]; i++)
			{
				const char *info;
				switch(*p++)
				{
				case 0:
					info = "Title";
					break;
				case 1:
					info = "Publisher";
					break;
				case 2:
					info = "Author";
					break;
				case 3:
					info = "Year";
					break;
				case 4:
					info = "Language";
					break;
				case 5:
					info = "Type";
					break;
				case 6:
					info = "Price";
					break;
				case 7:
					info = "Protection";
					break;
				case 8:
					info = "Origin";
					break;
				case -1:
					info = "Comment";
					break;
				default:
					info = "info";
					break;
				}
				dword size = *(byte*)p + 1;
				char tmp = p[size];
				p[size] = 0;
				sprintf(nm, "%s: %s", info, p + 1);
				p[size] = tmp;
				p += size;
				NamedCell(nm);
			}
			NamedCell("-");
			ptr += 2 + *(word*)ptr;
			break;
		case 0x33: // hardware type
			ParseHardware(ptr);
			ptr += 1 + 3 * *ptr;
			break;
		case 0x34: // emulation info
			NamedCell("* emulation info");
			ptr += 8;
			break;
		case 0x35: // custom info
			if(!memcmp(ptr, "POKEs           ", 16))
			{
				NamedCell("- POKEs block ");
				NamedCell(ptr + 0x15, ptr[0x14]);
				p = (char*)ptr + 0x15 + ptr[0x14];
				n = *(byte*)p++;
				for(i = 0; i < n; i++)
				{
					NamedCell(p + 1, *(byte*)p);
					p += *p + 1;
					t = *(byte*)p++;
					strcpy(nm, "POKE ");
					for(j = 0; j < t; j++)
					{
						sprintf(nm + strlen(nm), "%d,", *(word*)(p + 1));
						sprintf(nm + strlen(nm), *p & 0x10 ? "nn" : "%d",
								*(byte*)(p + 3));
						if(!(*p & 0x08))
							sprintf(nm + strlen(nm), "(page %d)", *p & 7);
						strcat(nm, "; ");
						p += 5;
					}
					NamedCell(nm);
				}
				nm[0] = '-';
				nm[1] = 0;
				nm[2] = 0;
				nm[3] = 0;
			}
			else
				sprintf(nm, "* custom info: %s", ptr), nm[15 + 16] = 0;
			NamedCell(nm);
			ptr += 0x14 + *(dword*)(ptr + 0x10);
			break;
		case 0x40: // snapshot
			NamedCell("* snapshot");
			ptr += 4 + (0xFFFFFF & *(dword*)(ptr + 1));
			break;
		case 0x5A: // 'Z'
			ptr += 9;
			break;
		default:
			ptr += buf_size;
		}
	}
	for(i = 0; i < tape_infosize; i++)
	{
		if(tapeinfo[i].desc[0] == '*' && tapeinfo[i].desc[1] == ' ')
			strcat(tapeinfo[i].desc, " [UNSUPPORTED]");
		if(*tapeinfo[i].desc == '-')
			while(strlen(tapeinfo[i].desc) < sizeof(tapeinfo[i].desc) - 1)
				strcat(tapeinfo[i].desc, "-");
	}
	if(tape_imagesize && tape_pulse[tape_image[tape_imagesize - 1]] < 350000)
		Reserve(1), tape_image[tape_imagesize++] = FindPulse(350000); // small pause [rqd for 3ddeathchase]
	FindTapeSizes();
	return (ptr == buf + buf_size);
}

byte eTape::TapeBit(int tact)
{
	qword cur = speccy->T() + tact;
	if(cur < tape.edge_change)
		return (byte)tape.tape_bit;
	while(tape.edge_change < cur)
	{
		dword t = (dword)(tape.edge_change - speccy->T());
		if((int)t >= 0)
		{
			const short vol = 2048;
			short mono = tape.tape_bit ? vol : 0;
			Update(tact, mono, mono);
		}
		dword pulse;
		tape.tape_bit ^= -1;
		if(tape.play_pointer == tape.end_of_tape || (pulse
				= tape_pulse[*tape.play_pointer++]) == (dword)-1)
			StopTape();
		else
			tape.edge_change += pulse;
	}
	return (byte)tape.tape_bit;
}

class eZ80_FastTape: public xZ80::eZ80
{
public:
	void Emul();
};

void FastTapeEmul(xZ80::eZ80* z80)
{
	((eZ80_FastTape*)z80)->Emul();
}

void eZ80_FastTape::Emul()
{
	byte p0 = Read(pc + 0);
	byte p1 = Read(pc + 1);
	byte p2 = Read(pc + 2);
	byte p3 = Read(pc + 3);
	if(p0 == 0x3D && p1 == 0x20 && p2 == 0xFD && p3 == 0xA7)
	{ // dec a:jr nz,$-1
		t += ((byte)(a - 1)) * 16;
		a = 1;
		return;
	}
	if(p0 == 0x10 && p1 == 0xFE)
	{ // djnz $
		t += ((byte)(b - 1)) * 13;
		b = 1;
		return;
	}
	if(p0 == 0x3D && p1 == 0xC2 && pc == dword(p3) * 0x100 + p2)
	{ // dec a:jp nz,$-1
		t += ((byte)(a - 1)) * 14;
		a = 1;
		return;
	}
	if(p0 == 0x04 && p1 == 0xC8 && p2 == 0x3E)
	{
		byte p04 = Read(pc + 4);
		byte p05 = Read(pc + 5);
		byte p06 = Read(pc + 6);
		byte p07 = Read(pc + 7);
		byte p08 = Read(pc + 8);
		byte p09 = Read(pc + 9);
		byte p10 = Read(pc + 10);
		byte p11 = Read(pc + 11);
		byte p12 = Read(pc + 12);
		if(p04 == 0xDB && p05 == 0xFE && p06 == 0x1F && p07 == 0xD0 && p08
				== 0xA9 && p09 == 0xE6 && p10 == 0x20 && p11 == 0x28 && p12
				== 0xF3)
		{ // find edge (rom routine)
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x20)
					return;
				b++;
				t += 59;
			}
		}
		if(p04 == 0xDB && p05 == 0xFE && p06 == 0xCB && p07 == 0x1F && p08
				== 0xA9 && p09 == 0xE6 && p10 == 0x20 && p11 == 0x28 && p12
				== 0xF3)
		{ // rra,ret nc => rr a (popeye2)
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x20)
					return;
				b++;
				t += 58;
			}
		}
		if(p04 == 0xDB && p05 == 0xFE && p06 == 0x1F && p07 == 0x00 && p08
				== 0xA9 && p09 == 0xE6 && p10 == 0x20 && p11 == 0x28 && p12
				== 0xF3)
		{ // ret nc nopped (some bleep loaders)
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x20)
					return;
				b++;
				t += 58;
			}
		}
		if(p04 == 0xDB && p05 == 0xFE && p06 == 0xA9 && p07 == 0xE6 && p08
				== 0x40 && p09 == 0xD8 && p10 == 0x00 && p11 == 0x28 && p12
				== 0xF3)
		{ // no rra, no break check (rana rama)
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x40)
					return;
				b++;
				t += 59;
			}
		}
		if(p04 == 0xDB && p05 == 0xFE && p06 == 0x1F && p07 == 0xA9 && p08
				== 0xE6 && p09 == 0x20 && p10 == 0x28 && p11 == 0xF4)
		{ // ret nc skipped: routine without BREAK checking (ZeroMusic & JSW)
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x20)
					return;
				b++;
				t += 54;
			}
		}
	}
	if(p0 == 0x04 && p1 == 0x20 && p2 == 0x03)
	{
		byte p06 = Read(pc + 6);
		byte p08 = Read(pc + 8);
		byte p09 = Read(pc + 9);
		byte p10 = Read(pc + 10);
		byte p11 = Read(pc + 11);
		byte p12 = Read(pc + 12);
		byte p13 = Read(pc + 13);
		byte p14 = Read(pc + 14);
		if(p06 == 0xDB && p08 == 0x1F && p09 == 0xC8 && p10 == 0xA9 && p11
				== 0xE6 && p12 == 0x20 && p13 == 0x28 && p14 == 0xF1)
		{ // find edge from Donkey Kong
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 0xFF)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x20)
					return;
				b++;
				t += 59;
			}
		}
	}
	if(p0 == 0x3E && p2 == 0xDB && p3 == 0xFE)
	{
		byte p04 = Read(pc + 4);
		byte p05 = Read(pc + 5);
		byte p06 = Read(pc + 6);
		byte p07 = Read(pc + 7);
		byte p09 = Read(pc + 9);
		byte p10 = Read(pc + 10);
		byte p11 = Read(pc + 11);
		if(p04 == 0xA9 && p05 == 0xE6 && p06 == 0x40 && p07 == 0x20 && p09
				== 0x05 && p10 == 0x20 && p11 == 0xF4)
		{ // lode runner
			eTape* tape = devices->Get<eTape> ();
			for(;;)
			{
				if(b == 1)
					return;
				if((tape->TapeBit(T()) ^ c) & 0x40)
					return;
				t += 52;
				b--;
			}
		}
	}
}
