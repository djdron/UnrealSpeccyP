#ifndef	__Z80_TABLES_H__
#define	__Z80_TABLES_H__

#pragma once

namespace xZ80
{

class eZ80;

void InitTables();

extern inline void inc8(eZ80* cpu, byte& x);
extern inline void dec8(eZ80* cpu, byte& x);
extern inline void add8(eZ80* cpu, byte src);
extern inline void adc8(eZ80* cpu, byte src);
extern inline void sub8(eZ80* cpu, byte src);
extern inline void sbc8(eZ80* cpu, byte src);
extern inline void and8(eZ80* cpu, byte src);
extern inline void or8(eZ80* cpu, byte src);
extern inline void xor8(eZ80* cpu, byte src);
extern inline void bit(eZ80* cpu, byte src, byte bit);
extern inline void bitmem(eZ80* cpu, byte src, byte bit);
extern inline void res(byte& src, byte bit);
extern inline byte resbyte(byte src, byte bit);
extern inline void set(byte& src, byte bit);
extern inline byte setbyte(byte src, byte bit);
extern inline void cp8(eZ80* cpu, byte src);

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
