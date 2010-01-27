#ifndef	__Z80_TABLES_H__
#define	__Z80_TABLES_H__

#pragma once

namespace xZ80
{

extern const byte* daatab;
extern const byte* incf;
extern const byte* decf;
extern const byte* rlcf;
extern const byte* rrcf;
extern const byte* rl0;
extern const byte* rl1;
extern const byte* rr0;
extern const byte* rr1;
extern const byte* sraf;

extern const byte* adcf;	// flags for adc and add
extern const byte* sbcf;	// flags for sub and sbc
extern const byte* cpf;		// flags for cp
extern const byte* cpf8b;	// flags for CPD/CPI/CPDR/CPIR
extern const byte* log_f;
extern const byte* rlcaf;
extern const byte* rrcaf;
extern const byte* rol;
extern const byte* ror;

}//namespace xZ80

#endif//__Z80_TABLES_H__
