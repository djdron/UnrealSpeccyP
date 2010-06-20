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

#include "../std.h"

#include "z80.h"
#include "../devices/memory.h"
#include "../devices/ula.h"
#include "../devices/device.h"

namespace xZ80
{

//=============================================================================
//	eZ80::IoWrite
//-----------------------------------------------------------------------------
void eZ80::IoWrite(word port, byte v)
{
	devices->IoWrite(port, v, t);
}
//=============================================================================
//	eZ80::IoRead
//-----------------------------------------------------------------------------
byte eZ80::IoRead(word port) const
{
	return devices->IoRead(port, t);
}
//=============================================================================
//	eZ80::Write
//-----------------------------------------------------------------------------
void eZ80::Write(word addr, byte v)
{
	ula->Write(t);
	memory->Write(addr, v);
}
//=============================================================================
//	eZ80::Read
//-----------------------------------------------------------------------------
inline byte eZ80::Read(word addr) const
{
	return memory->Read(addr);
}

//=============================================================================
//	eZ80::InitOpNoPrefix
//-----------------------------------------------------------------------------
void eZ80::InitOpNoPrefix()
{
	const CALLFUNC opcodes[] =
	{
		&eZ80::Op00, &eZ80::Op01, &eZ80::Op02, &eZ80::Op03, &eZ80::Op04, &eZ80::Op05, &eZ80::Op06, &eZ80::Op07,
		&eZ80::Op08, &eZ80::Op09, &eZ80::Op0A, &eZ80::Op0B, &eZ80::Op0C, &eZ80::Op0D, &eZ80::Op0E, &eZ80::Op0F,
		&eZ80::Op10, &eZ80::Op11, &eZ80::Op12, &eZ80::Op13, &eZ80::Op14, &eZ80::Op15, &eZ80::Op16, &eZ80::Op17,
		&eZ80::Op18, &eZ80::Op19, &eZ80::Op1A, &eZ80::Op1B, &eZ80::Op1C, &eZ80::Op1D, &eZ80::Op1E, &eZ80::Op1F,
		&eZ80::Op20, &eZ80::Op21, &eZ80::Op22, &eZ80::Op23, &eZ80::Op24, &eZ80::Op25, &eZ80::Op26, &eZ80::Op27,
		&eZ80::Op28, &eZ80::Op29, &eZ80::Op2A, &eZ80::Op2B, &eZ80::Op2C, &eZ80::Op2D, &eZ80::Op2E, &eZ80::Op2F,
		&eZ80::Op30, &eZ80::Op31, &eZ80::Op32, &eZ80::Op33, &eZ80::Op34, &eZ80::Op35, &eZ80::Op36, &eZ80::Op37,
		&eZ80::Op38, &eZ80::Op39, &eZ80::Op3A, &eZ80::Op3B, &eZ80::Op3C, &eZ80::Op3D, &eZ80::Op3E, &eZ80::Op3F,

		&eZ80::Op40, &eZ80::Op41, &eZ80::Op42, &eZ80::Op43, &eZ80::Op44, &eZ80::Op45, &eZ80::Op46, &eZ80::Op47,
		&eZ80::Op48, &eZ80::Op49, &eZ80::Op4A, &eZ80::Op4B, &eZ80::Op4C, &eZ80::Op4D, &eZ80::Op4E, &eZ80::Op4F,
		&eZ80::Op50, &eZ80::Op51, &eZ80::Op52, &eZ80::Op53, &eZ80::Op54, &eZ80::Op55, &eZ80::Op56, &eZ80::Op57,
		&eZ80::Op58, &eZ80::Op59, &eZ80::Op5A, &eZ80::Op5B, &eZ80::Op5C, &eZ80::Op5D, &eZ80::Op5E, &eZ80::Op5F,
		&eZ80::Op60, &eZ80::Op61, &eZ80::Op62, &eZ80::Op63, &eZ80::Op64, &eZ80::Op65, &eZ80::Op66, &eZ80::Op67,
		&eZ80::Op68, &eZ80::Op69, &eZ80::Op6A, &eZ80::Op6B, &eZ80::Op6C, &eZ80::Op6D, &eZ80::Op6E, &eZ80::Op6F,
		&eZ80::Op70, &eZ80::Op71, &eZ80::Op72, &eZ80::Op73, &eZ80::Op74, &eZ80::Op75, &eZ80::Op76, &eZ80::Op77,
		&eZ80::Op78, &eZ80::Op79, &eZ80::Op7A, &eZ80::Op7B, &eZ80::Op7C, &eZ80::Op7D, &eZ80::Op7E, &eZ80::Op7F,

		&eZ80::Op80, &eZ80::Op81, &eZ80::Op82, &eZ80::Op83, &eZ80::Op84, &eZ80::Op85, &eZ80::Op86, &eZ80::Op87,
		&eZ80::Op88, &eZ80::Op89, &eZ80::Op8A, &eZ80::Op8B, &eZ80::Op8C, &eZ80::Op8D, &eZ80::Op8E, &eZ80::Op8F,
		&eZ80::Op90, &eZ80::Op91, &eZ80::Op92, &eZ80::Op93, &eZ80::Op94, &eZ80::Op95, &eZ80::Op96, &eZ80::Op97,
		&eZ80::Op98, &eZ80::Op99, &eZ80::Op9A, &eZ80::Op9B, &eZ80::Op9C, &eZ80::Op9D, &eZ80::Op9E, &eZ80::Op9F,
		&eZ80::OpA0, &eZ80::OpA1, &eZ80::OpA2, &eZ80::OpA3, &eZ80::OpA4, &eZ80::OpA5, &eZ80::OpA6, &eZ80::OpA7,
		&eZ80::OpA8, &eZ80::OpA9, &eZ80::OpAA, &eZ80::OpAB, &eZ80::OpAC, &eZ80::OpAD, &eZ80::OpAE, &eZ80::OpAF,
		&eZ80::OpB0, &eZ80::OpB1, &eZ80::OpB2, &eZ80::OpB3, &eZ80::OpB4, &eZ80::OpB5, &eZ80::OpB6, &eZ80::OpB7,
		&eZ80::OpB8, &eZ80::OpB9, &eZ80::OpBA, &eZ80::OpBB, &eZ80::OpBC, &eZ80::OpBD, &eZ80::OpBE, &eZ80::OpBF,

		&eZ80::OpC0, &eZ80::OpC1, &eZ80::OpC2, &eZ80::OpC3, &eZ80::OpC4, &eZ80::OpC5, &eZ80::OpC6, &eZ80::OpC7,
		&eZ80::OpC8, &eZ80::OpC9, &eZ80::OpCA, &eZ80::OpCB, &eZ80::OpCC, &eZ80::OpCD, &eZ80::OpCE, &eZ80::OpCF,
		&eZ80::OpD0, &eZ80::OpD1, &eZ80::OpD2, &eZ80::OpD3, &eZ80::OpD4, &eZ80::OpD5, &eZ80::OpD6, &eZ80::OpD7,
		&eZ80::OpD8, &eZ80::OpD9, &eZ80::OpDA, &eZ80::OpDB, &eZ80::OpDC, &eZ80::OpDD, &eZ80::OpDE, &eZ80::OpDF,
		&eZ80::OpE0, &eZ80::OpE1, &eZ80::OpE2, &eZ80::OpE3, &eZ80::OpE4, &eZ80::OpE5, &eZ80::OpE6, &eZ80::OpE7,
		&eZ80::OpE8, &eZ80::OpE9, &eZ80::OpEA, &eZ80::OpEB, &eZ80::OpEC, &eZ80::OpED, &eZ80::OpEE, &eZ80::OpEF,
		&eZ80::OpF0, &eZ80::OpF1, &eZ80::OpF2, &eZ80::OpF3, &eZ80::OpF4, &eZ80::OpF5, &eZ80::OpF6, &eZ80::OpF7,
		&eZ80::OpF8, &eZ80::OpF9, &eZ80::OpFA, &eZ80::OpFB, &eZ80::OpFC, &eZ80::OpFD, &eZ80::OpFE, &eZ80::OpFF
	};
	memcpy(normal_opcodes, opcodes, sizeof(opcodes));
}
//=============================================================================
//	eZ80::InitOpCB
//-----------------------------------------------------------------------------
void eZ80::InitOpCB()
{
	const CALLFUNC opcodes[] =
	{
		&eZ80::Opl00, &eZ80::Opl01, &eZ80::Opl02, &eZ80::Opl03, &eZ80::Opl04, &eZ80::Opl05, &eZ80::Opl06, &eZ80::Opl07,
		&eZ80::Opl08, &eZ80::Opl09, &eZ80::Opl0A, &eZ80::Opl0B, &eZ80::Opl0C, &eZ80::Opl0D, &eZ80::Opl0E, &eZ80::Opl0F,
		&eZ80::Opl10, &eZ80::Opl11, &eZ80::Opl12, &eZ80::Opl13, &eZ80::Opl14, &eZ80::Opl15, &eZ80::Opl16, &eZ80::Opl17,
		&eZ80::Opl18, &eZ80::Opl19, &eZ80::Opl1A, &eZ80::Opl1B, &eZ80::Opl1C, &eZ80::Opl1D, &eZ80::Opl1E, &eZ80::Opl1F,
		&eZ80::Opl20, &eZ80::Opl21, &eZ80::Opl22, &eZ80::Opl23, &eZ80::Opl24, &eZ80::Opl25, &eZ80::Opl26, &eZ80::Opl27,
		&eZ80::Opl28, &eZ80::Opl29, &eZ80::Opl2A, &eZ80::Opl2B, &eZ80::Opl2C, &eZ80::Opl2D, &eZ80::Opl2E, &eZ80::Opl2F,
		&eZ80::Opl30, &eZ80::Opl31, &eZ80::Opl32, &eZ80::Opl33, &eZ80::Opl34, &eZ80::Opl35, &eZ80::Opl36, &eZ80::Opl37,
		&eZ80::Opl38, &eZ80::Opl39, &eZ80::Opl3A, &eZ80::Opl3B, &eZ80::Opl3C, &eZ80::Opl3D, &eZ80::Opl3E, &eZ80::Opl3F,

		&eZ80::Opl40, &eZ80::Opl41, &eZ80::Opl42, &eZ80::Opl43, &eZ80::Opl44, &eZ80::Opl45, &eZ80::Opl46, &eZ80::Opl47,
		&eZ80::Opl48, &eZ80::Opl49, &eZ80::Opl4A, &eZ80::Opl4B, &eZ80::Opl4C, &eZ80::Opl4D, &eZ80::Opl4E, &eZ80::Opl4F,
		&eZ80::Opl50, &eZ80::Opl51, &eZ80::Opl52, &eZ80::Opl53, &eZ80::Opl54, &eZ80::Opl55, &eZ80::Opl56, &eZ80::Opl57,
		&eZ80::Opl58, &eZ80::Opl59, &eZ80::Opl5A, &eZ80::Opl5B, &eZ80::Opl5C, &eZ80::Opl5D, &eZ80::Opl5E, &eZ80::Opl5F,
		&eZ80::Opl60, &eZ80::Opl61, &eZ80::Opl62, &eZ80::Opl63, &eZ80::Opl64, &eZ80::Opl65, &eZ80::Opl66, &eZ80::Opl67,
		&eZ80::Opl68, &eZ80::Opl69, &eZ80::Opl6A, &eZ80::Opl6B, &eZ80::Opl6C, &eZ80::Opl6D, &eZ80::Opl6E, &eZ80::Opl6F,
		&eZ80::Opl70, &eZ80::Opl71, &eZ80::Opl72, &eZ80::Opl73, &eZ80::Opl74, &eZ80::Opl75, &eZ80::Opl76, &eZ80::Opl77,
		&eZ80::Opl78, &eZ80::Opl79, &eZ80::Opl7A, &eZ80::Opl7B, &eZ80::Opl7C, &eZ80::Opl7D, &eZ80::Opl7E, &eZ80::Opl7F,

		&eZ80::Opl80, &eZ80::Opl81, &eZ80::Opl82, &eZ80::Opl83, &eZ80::Opl84, &eZ80::Opl85, &eZ80::Opl86, &eZ80::Opl87,
		&eZ80::Opl88, &eZ80::Opl89, &eZ80::Opl8A, &eZ80::Opl8B, &eZ80::Opl8C, &eZ80::Opl8D, &eZ80::Opl8E, &eZ80::Opl8F,
		&eZ80::Opl90, &eZ80::Opl91, &eZ80::Opl92, &eZ80::Opl93, &eZ80::Opl94, &eZ80::Opl95, &eZ80::Opl96, &eZ80::Opl97,
		&eZ80::Opl98, &eZ80::Opl99, &eZ80::Opl9A, &eZ80::Opl9B, &eZ80::Opl9C, &eZ80::Opl9D, &eZ80::Opl9E, &eZ80::Opl9F,
		&eZ80::OplA0, &eZ80::OplA1, &eZ80::OplA2, &eZ80::OplA3, &eZ80::OplA4, &eZ80::OplA5, &eZ80::OplA6, &eZ80::OplA7,
		&eZ80::OplA8, &eZ80::OplA9, &eZ80::OplAA, &eZ80::OplAB, &eZ80::OplAC, &eZ80::OplAD, &eZ80::OplAE, &eZ80::OplAF,
		&eZ80::OplB0, &eZ80::OplB1, &eZ80::OplB2, &eZ80::OplB3, &eZ80::OplB4, &eZ80::OplB5, &eZ80::OplB6, &eZ80::OplB7,
		&eZ80::OplB8, &eZ80::OplB9, &eZ80::OplBA, &eZ80::OplBB, &eZ80::OplBC, &eZ80::OplBD, &eZ80::OplBE, &eZ80::OplBF,

		&eZ80::OplC0, &eZ80::OplC1, &eZ80::OplC2, &eZ80::OplC3, &eZ80::OplC4, &eZ80::OplC5, &eZ80::OplC6, &eZ80::OplC7,
		&eZ80::OplC8, &eZ80::OplC9, &eZ80::OplCA, &eZ80::OplCB, &eZ80::OplCC, &eZ80::OplCD, &eZ80::OplCE, &eZ80::OplCF,
		&eZ80::OplD0, &eZ80::OplD1, &eZ80::OplD2, &eZ80::OplD3, &eZ80::OplD4, &eZ80::OplD5, &eZ80::OplD6, &eZ80::OplD7,
		&eZ80::OplD8, &eZ80::OplD9, &eZ80::OplDA, &eZ80::OplDB, &eZ80::OplDC, &eZ80::OplDD, &eZ80::OplDE, &eZ80::OplDF,
		&eZ80::OplE0, &eZ80::OplE1, &eZ80::OplE2, &eZ80::OplE3, &eZ80::OplE4, &eZ80::OplE5, &eZ80::OplE6, &eZ80::OplE7,
		&eZ80::OplE8, &eZ80::OplE9, &eZ80::OplEA, &eZ80::OplEB, &eZ80::OplEC, &eZ80::OplED, &eZ80::OplEE, &eZ80::OplEF,
		&eZ80::OplF0, &eZ80::OplF1, &eZ80::OplF2, &eZ80::OplF3, &eZ80::OplF4, &eZ80::OplF5, &eZ80::OplF6, &eZ80::OplF7,
		&eZ80::OplF8, &eZ80::OplF9, &eZ80::OplFA, &eZ80::OplFB, &eZ80::OplFC, &eZ80::OplFD, &eZ80::OplFE, &eZ80::OplFF
	};
	memcpy(logic_opcodes, opcodes, sizeof(opcodes));
}
//=============================================================================
//	eZ80::InitOpDD
//-----------------------------------------------------------------------------
void eZ80::InitOpDD()
{
	CALLFUNC const opcodes[] = 
	{
		&eZ80::Op00 , &eZ80::Op01 , &eZ80::Op02 , &eZ80::Op03 , &eZ80::Op04 , &eZ80::Op05 , &eZ80::Op06 , &eZ80::Op07 ,
		&eZ80::Op08 , &eZ80::Opx09, &eZ80::Op0A , &eZ80::Op0B , &eZ80::Op0C , &eZ80::Op0D , &eZ80::Op0E , &eZ80::Op0F ,
		&eZ80::Op10 , &eZ80::Op11 , &eZ80::Op12 , &eZ80::Op13 , &eZ80::Op14 , &eZ80::Op15 , &eZ80::Op16 , &eZ80::Op17 ,
		&eZ80::Op18 , &eZ80::Opx19, &eZ80::Op1A , &eZ80::Op1B , &eZ80::Op1C , &eZ80::Op1D , &eZ80::Op1E , &eZ80::Op1F ,
		&eZ80::Op20 , &eZ80::Opx21, &eZ80::Opx22, &eZ80::Opx23, &eZ80::Opx24, &eZ80::Opx25, &eZ80::Opx26, &eZ80::Op27 ,
		&eZ80::Op28 , &eZ80::Opx29, &eZ80::Opx2A, &eZ80::Opx2B, &eZ80::Opx2C, &eZ80::Opx2D, &eZ80::Opx2E, &eZ80::Op2F ,
		&eZ80::Op30 , &eZ80::Op31 , &eZ80::Op32 , &eZ80::Op33 , &eZ80::Opx34, &eZ80::Opx35, &eZ80::Opx36, &eZ80::Op37 ,
		&eZ80::Op38 , &eZ80::Opx39, &eZ80::Op3A , &eZ80::Op3B , &eZ80::Op3C , &eZ80::Op3D , &eZ80::Op3E , &eZ80::Op3F ,

		&eZ80::Op40 , &eZ80::Op41 , &eZ80::Op42 , &eZ80::Op43 , &eZ80::Opx44, &eZ80::Opx45, &eZ80::Opx46, &eZ80::Op47 ,
		&eZ80::Op48 , &eZ80::Op49 , &eZ80::Op4A , &eZ80::Op4B , &eZ80::Opx4C, &eZ80::Opx4D, &eZ80::Opx4E, &eZ80::Op4F ,
		&eZ80::Op50 , &eZ80::Op51 , &eZ80::Op52 , &eZ80::Op53 , &eZ80::Opx54, &eZ80::Opx55, &eZ80::Opx56, &eZ80::Op57 ,
		&eZ80::Op58 , &eZ80::Op59 , &eZ80::Op5A , &eZ80::Op5B , &eZ80::Opx5C, &eZ80::Opx5D, &eZ80::Opx5E, &eZ80::Op5F ,
		&eZ80::Opx60, &eZ80::Opx61, &eZ80::Opx62, &eZ80::Opx63, &eZ80::Op64 , &eZ80::Opx65, &eZ80::Opx66, &eZ80::Opx67,
		&eZ80::Opx68, &eZ80::Opx69, &eZ80::Opx6A, &eZ80::Opx6B, &eZ80::Opx6C, &eZ80::Op6D , &eZ80::Opx6E, &eZ80::Opx6F,
		&eZ80::Opx70, &eZ80::Opx71, &eZ80::Opx72, &eZ80::Opx73, &eZ80::Opx74, &eZ80::Opx75, &eZ80::Op76 , &eZ80::Opx77,
		&eZ80::Op78 , &eZ80::Op79 , &eZ80::Op7A , &eZ80::Op7B , &eZ80::Opx7C, &eZ80::Opx7D, &eZ80::Opx7E, &eZ80::Op7F ,

		&eZ80::Op80 , &eZ80::Op81 , &eZ80::Op82 , &eZ80::Op83 , &eZ80::Opx84, &eZ80::Opx85, &eZ80::Opx86, &eZ80::Op87 ,
		&eZ80::Op88 , &eZ80::Op89 , &eZ80::Op8A , &eZ80::Op8B , &eZ80::Opx8C, &eZ80::Opx8D, &eZ80::Opx8E, &eZ80::Op8F ,
		&eZ80::Op90 , &eZ80::Op91 , &eZ80::Op92 , &eZ80::Op93 , &eZ80::Opx94, &eZ80::Opx95, &eZ80::Opx96, &eZ80::Op97 ,
		&eZ80::Op98 , &eZ80::Op99 , &eZ80::Op9A , &eZ80::Op9B , &eZ80::Opx9C, &eZ80::Opx9D, &eZ80::Opx9E, &eZ80::Op9F ,
		&eZ80::OpA0 , &eZ80::OpA1 , &eZ80::OpA2 , &eZ80::OpA3 , &eZ80::OpxA4, &eZ80::OpxA5, &eZ80::OpxA6, &eZ80::OpA7 ,
		&eZ80::OpA8 , &eZ80::OpA9 , &eZ80::OpAA , &eZ80::OpAB , &eZ80::OpxAC, &eZ80::OpxAD, &eZ80::OpxAE, &eZ80::OpAF ,
		&eZ80::OpB0 , &eZ80::OpB1 , &eZ80::OpB2 , &eZ80::OpB3 , &eZ80::OpxB4, &eZ80::OpxB5, &eZ80::OpxB6, &eZ80::OpB7 ,
		&eZ80::OpB8 , &eZ80::OpB9 , &eZ80::OpBA , &eZ80::OpBB , &eZ80::OpxBC, &eZ80::OpxBD, &eZ80::OpxBE, &eZ80::OpBF ,

		&eZ80::OpC0 , &eZ80::OpC1 , &eZ80::OpC2 , &eZ80::OpC3 , &eZ80::OpC4 , &eZ80::OpC5 , &eZ80::OpC6 , &eZ80::OpC7 ,
		&eZ80::OpC8 , &eZ80::OpC9 , &eZ80::OpCA , &eZ80::OpCB , &eZ80::OpCC , &eZ80::OpCD , &eZ80::OpCE , &eZ80::OpCF ,
		&eZ80::OpD0 , &eZ80::OpD1 , &eZ80::OpD2 , &eZ80::OpD3 , &eZ80::OpD4 , &eZ80::OpD5 , &eZ80::OpD6 , &eZ80::OpD7 ,
		&eZ80::OpD8 , &eZ80::OpD9 , &eZ80::OpDA , &eZ80::OpDB , &eZ80::OpDC , &eZ80::OpDD , &eZ80::OpDE , &eZ80::OpDF ,
		&eZ80::OpE0 , &eZ80::OpxE1, &eZ80::OpE2 , &eZ80::OpxE3, &eZ80::OpE4 , &eZ80::OpxE5, &eZ80::OpE6 , &eZ80::OpE7 ,
		&eZ80::OpE8 , &eZ80::OpxE9, &eZ80::OpEA , &eZ80::OpEB , &eZ80::OpEC , &eZ80::OpED , &eZ80::OpEE , &eZ80::OpEF ,
		&eZ80::OpF0 , &eZ80::OpF1 , &eZ80::OpF2 , &eZ80::OpF3 , &eZ80::OpF4 , &eZ80::OpF5 , &eZ80::OpF6 , &eZ80::OpF7 ,
		&eZ80::OpF8 , &eZ80::OpxF9, &eZ80::OpFA , &eZ80::OpFB , &eZ80::OpFC , &eZ80::OpFD , &eZ80::OpFE , &eZ80::OpFF
	};
	memcpy(ix_opcodes, opcodes, sizeof(opcodes));
}
//=============================================================================
//	eZ80::InitOpED
//-----------------------------------------------------------------------------
void eZ80::InitOpED()
{
	CALLFUNC const opcodes[] = 
	{
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,

		&eZ80::Ope40, &eZ80::Ope41, &eZ80::Ope42, &eZ80::Ope43, &eZ80::Ope44, &eZ80::Ope45, &eZ80::Ope46, &eZ80::Ope47,
		&eZ80::Ope48, &eZ80::Ope49, &eZ80::Ope4A, &eZ80::Ope4B, &eZ80::Ope4C, &eZ80::Ope4D, &eZ80::Ope4E, &eZ80::Ope4F,
		&eZ80::Ope50, &eZ80::Ope51, &eZ80::Ope52, &eZ80::Ope53, &eZ80::Ope54, &eZ80::Ope55, &eZ80::Ope56, &eZ80::Ope57,
		&eZ80::Ope58, &eZ80::Ope59, &eZ80::Ope5A, &eZ80::Ope5B, &eZ80::Ope5C, &eZ80::Ope5D, &eZ80::Ope5E, &eZ80::Ope5F,
		&eZ80::Ope60, &eZ80::Ope61, &eZ80::Ope62, &eZ80::Ope63, &eZ80::Ope64, &eZ80::Ope65, &eZ80::Ope66, &eZ80::Ope67,
		&eZ80::Ope68, &eZ80::Ope69, &eZ80::Ope6A, &eZ80::Ope6B, &eZ80::Ope6C, &eZ80::Ope6D, &eZ80::Ope6E, &eZ80::Ope6F,
		&eZ80::Ope70, &eZ80::Ope71, &eZ80::Ope72, &eZ80::Ope73, &eZ80::Ope74, &eZ80::Ope75, &eZ80::Ope76, &eZ80::Ope77,
		&eZ80::Ope78, &eZ80::Ope79, &eZ80::Ope7A, &eZ80::Ope7B, &eZ80::Ope7C, &eZ80::Ope7D, &eZ80::Ope7E, &eZ80::Ope7F,

		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::OpeA0, &eZ80::OpeA1, &eZ80::OpeA2, &eZ80::OpeA3, &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::OpeA8, &eZ80::OpeA9, &eZ80::OpeAA, &eZ80::OpeAB, &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::OpeB0, &eZ80::OpeB1, &eZ80::OpeB2, &eZ80::OpeB3, &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::OpeB8, &eZ80::OpeB9, &eZ80::OpeBA, &eZ80::OpeBB, &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,

		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 ,
		&eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00 , &eZ80::Op00
	};
	memcpy(ext_opcodes, opcodes, sizeof(opcodes));
}
//=============================================================================
//	eZ80::InitOpFD
//-----------------------------------------------------------------------------
void eZ80::InitOpFD()
{
	CALLFUNC const opcodes[] =
	{
		&eZ80::Op00 , &eZ80::Op01 , &eZ80::Op02 , &eZ80::Op03 , &eZ80::Op04 , &eZ80::Op05 , &eZ80::Op06 , &eZ80::Op07 ,
		&eZ80::Op08 , &eZ80::Opy09, &eZ80::Op0A , &eZ80::Op0B , &eZ80::Op0C , &eZ80::Op0D , &eZ80::Op0E , &eZ80::Op0F ,
		&eZ80::Op10 , &eZ80::Op11 , &eZ80::Op12 , &eZ80::Op13 , &eZ80::Op14 , &eZ80::Op15 , &eZ80::Op16 , &eZ80::Op17 ,
		&eZ80::Op18 , &eZ80::Opy19, &eZ80::Op1A , &eZ80::Op1B , &eZ80::Op1C , &eZ80::Op1D , &eZ80::Op1E , &eZ80::Op1F ,
		&eZ80::Op20 , &eZ80::Opy21, &eZ80::Opy22, &eZ80::Opy23, &eZ80::Opy24, &eZ80::Opy25, &eZ80::Opy26, &eZ80::Op27 ,
		&eZ80::Op28 , &eZ80::Opy29, &eZ80::Opy2A, &eZ80::Opy2B, &eZ80::Opy2C, &eZ80::Opy2D, &eZ80::Opy2E, &eZ80::Op2F ,
		&eZ80::Op30 , &eZ80::Op31 , &eZ80::Op32 , &eZ80::Op33 , &eZ80::Opy34, &eZ80::Opy35, &eZ80::Opy36, &eZ80::Op37 ,
		&eZ80::Op38 , &eZ80::Opy39, &eZ80::Op3A , &eZ80::Op3B , &eZ80::Op3C , &eZ80::Op3D , &eZ80::Op3E , &eZ80::Op3F ,

		&eZ80::Op40 , &eZ80::Op41 , &eZ80::Op42 , &eZ80::Op43 , &eZ80::Opy44, &eZ80::Opy45, &eZ80::Opy46, &eZ80::Op47 ,
		&eZ80::Op48 , &eZ80::Op49 , &eZ80::Op4A , &eZ80::Op4B , &eZ80::Opy4C, &eZ80::Opy4D, &eZ80::Opy4E, &eZ80::Op4F ,
		&eZ80::Op50 , &eZ80::Op51 , &eZ80::Op52 , &eZ80::Op53 , &eZ80::Opy54, &eZ80::Opy55, &eZ80::Opy56, &eZ80::Op57 ,
		&eZ80::Op58 , &eZ80::Op59 , &eZ80::Op5A , &eZ80::Op5B , &eZ80::Opy5C, &eZ80::Opy5D, &eZ80::Opy5E, &eZ80::Op5F ,
		&eZ80::Opy60, &eZ80::Opy61, &eZ80::Opy62, &eZ80::Opy63, &eZ80::Op64 , &eZ80::Opy65, &eZ80::Opy66, &eZ80::Opy67,
		&eZ80::Opy68, &eZ80::Opy69, &eZ80::Opy6A, &eZ80::Opy6B, &eZ80::Opy6C, &eZ80::Op6D , &eZ80::Opy6E, &eZ80::Opy6F,
		&eZ80::Opy70, &eZ80::Opy71, &eZ80::Opy72, &eZ80::Opy73, &eZ80::Opy74, &eZ80::Opy75, &eZ80::Op76 , &eZ80::Opy77,
		&eZ80::Op78 , &eZ80::Op79 , &eZ80::Op7A , &eZ80::Op7B , &eZ80::Opy7C, &eZ80::Opy7D, &eZ80::Opy7E, &eZ80::Op7F ,

		&eZ80::Op80 , &eZ80::Op81 , &eZ80::Op82 , &eZ80::Op83 , &eZ80::Opy84, &eZ80::Opy85, &eZ80::Opy86, &eZ80::Op87 ,
		&eZ80::Op88 , &eZ80::Op89 , &eZ80::Op8A , &eZ80::Op8B , &eZ80::Opy8C, &eZ80::Opy8D, &eZ80::Opy8E, &eZ80::Op8F ,
		&eZ80::Op90 , &eZ80::Op91 , &eZ80::Op92 , &eZ80::Op93 , &eZ80::Opy94, &eZ80::Opy95, &eZ80::Opy96, &eZ80::Op97 ,
		&eZ80::Op98 , &eZ80::Op99 , &eZ80::Op9A , &eZ80::Op9B , &eZ80::Opy9C, &eZ80::Opy9D, &eZ80::Opy9E, &eZ80::Op9F ,
		&eZ80::OpA0 , &eZ80::OpA1 , &eZ80::OpA2 , &eZ80::OpA3 , &eZ80::OpyA4, &eZ80::OpyA5, &eZ80::OpyA6, &eZ80::OpA7 ,
		&eZ80::OpA8 , &eZ80::OpA9 , &eZ80::OpAA , &eZ80::OpAB , &eZ80::OpyAC, &eZ80::OpyAD, &eZ80::OpyAE, &eZ80::OpAF ,
		&eZ80::OpB0 , &eZ80::OpB1 , &eZ80::OpB2 , &eZ80::OpB3 , &eZ80::OpyB4, &eZ80::OpyB5, &eZ80::OpyB6, &eZ80::OpB7 ,
		&eZ80::OpB8 , &eZ80::OpB9 , &eZ80::OpBA , &eZ80::OpBB , &eZ80::OpyBC, &eZ80::OpyBD, &eZ80::OpyBE, &eZ80::OpBF ,

		&eZ80::OpC0 , &eZ80::OpC1 , &eZ80::OpC2 , &eZ80::OpC3 , &eZ80::OpC4 , &eZ80::OpC5 , &eZ80::OpC6 , &eZ80::OpC7 ,
		&eZ80::OpC8 , &eZ80::OpC9 , &eZ80::OpCA , &eZ80::OpCB , &eZ80::OpCC , &eZ80::OpCD , &eZ80::OpCE , &eZ80::OpCF ,
		&eZ80::OpD0 , &eZ80::OpD1 , &eZ80::OpD2 , &eZ80::OpD3 , &eZ80::OpD4 , &eZ80::OpD5 , &eZ80::OpD6 , &eZ80::OpD7 ,
		&eZ80::OpD8 , &eZ80::OpD9 , &eZ80::OpDA , &eZ80::OpDB , &eZ80::OpDC , &eZ80::OpDD , &eZ80::OpDE , &eZ80::OpDF ,
		&eZ80::OpE0 , &eZ80::OpyE1, &eZ80::OpE2 , &eZ80::OpyE3, &eZ80::OpE4 , &eZ80::OpyE5, &eZ80::OpE6 , &eZ80::OpE7 ,
		&eZ80::OpE8 , &eZ80::OpyE9, &eZ80::OpEA , &eZ80::OpEB , &eZ80::OpEC , &eZ80::OpED , &eZ80::OpEE , &eZ80::OpEF ,
		&eZ80::OpF0 , &eZ80::OpF1 , &eZ80::OpF2 , &eZ80::OpF3 , &eZ80::OpF4 , &eZ80::OpF5 , &eZ80::OpF6 , &eZ80::OpF7 ,
		&eZ80::OpF8 , &eZ80::OpyF9, &eZ80::OpFA , &eZ80::OpFB , &eZ80::OpFC , &eZ80::OpFD , &eZ80::OpFE , &eZ80::OpFF
	};
	memcpy(iy_opcodes, opcodes, sizeof(opcodes));
}
//=============================================================================
//	eZ80::InitOpDDCB
//-----------------------------------------------------------------------------
void eZ80::InitOpDDCB()
{
	CALLFUNCI const opcodes[] =
	{
		&eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00, &eZ80::Oplx00,
		&eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08, &eZ80::Oplx08,
		&eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10, &eZ80::Oplx10,
		&eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18, &eZ80::Oplx18,
		&eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20, &eZ80::Oplx20,
		&eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28, &eZ80::Oplx28,
		&eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30, &eZ80::Oplx30,
		&eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38, &eZ80::Oplx38,

		&eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40, &eZ80::Oplx40,
		&eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48, &eZ80::Oplx48,
		&eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50, &eZ80::Oplx50,
		&eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58, &eZ80::Oplx58,
		&eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60, &eZ80::Oplx60,
		&eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68, &eZ80::Oplx68,
		&eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70, &eZ80::Oplx70,
		&eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78, &eZ80::Oplx78,

		&eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80, &eZ80::Oplx80,
		&eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88, &eZ80::Oplx88,
		&eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90, &eZ80::Oplx90,
		&eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98, &eZ80::Oplx98,
		&eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0, &eZ80::OplxA0,
		&eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8, &eZ80::OplxA8,
		&eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0, &eZ80::OplxB0,
		&eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8, &eZ80::OplxB8,

		&eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0, &eZ80::OplxC0,
		&eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8, &eZ80::OplxC8,
		&eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0, &eZ80::OplxD0,
		&eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8, &eZ80::OplxD8,
		&eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0, &eZ80::OplxE0,
		&eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8, &eZ80::OplxE8,
		&eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0, &eZ80::OplxF0,
		&eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8, &eZ80::OplxF8
	};
	memcpy(logic_ix_opcodes, opcodes, sizeof(opcodes));
}

}//namespace xZ80
