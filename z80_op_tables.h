#ifndef	__Z80_TABLES_H__
#define	__Z80_TABLES_H__

#pragma once

namespace xZ80
{

class eZ80;

void InitTables();

void inc8(eZ80* cpu, byte& x);
void dec8(eZ80* cpu, byte& x);
void add8(eZ80* cpu, byte src);
void adc8(eZ80* cpu, byte src);
void sub8(eZ80* cpu, byte src);
void sbc8(eZ80* cpu, byte src);
void and8(eZ80* cpu, byte src);
void or8(eZ80* cpu, byte src);
void xor8(eZ80* cpu, byte src);
void bit(eZ80* cpu, byte src, byte bit);
void bitmem(eZ80* cpu, byte src, byte bit);
void res(byte& src, byte bit);
byte resbyte(byte src, byte bit);
void set(byte& src, byte bit);
byte setbyte(byte src, byte bit);
void cp8(eZ80* cpu, byte src);

extern const byte daatab[];
extern const byte rlcf[];
extern const byte rrcf[];
extern const byte rl0[];
extern const byte rl1[];
extern const byte rr0[];
extern const byte rr1[];
extern const byte sraf[];

extern byte adcf[];	// flags for adc and add
extern byte sbcf[];	// flags for sub and sbc
extern byte cpf[];		// flags for cp
extern byte cpf8b[];	// flags for CPD/CPI/CPDR/CPIR
extern byte log_f[];
extern byte rlcaf[];
extern byte rrcaf[];
extern byte rol[];
extern byte ror[];

typedef byte (eZ80::*REGP);
extern REGP reg_offset[];

}//namespace xZ80

#endif//__Z80_TABLES_H__
