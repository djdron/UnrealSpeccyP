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

#ifdef _SYMBIAN

#if __GCCE__

extern "C"
{

extern unsigned int __aeabi_uidivmod(unsigned numerator, unsigned denominator);

int __aeabi_idiv(int numerator, int denominator)
{
	int neg_result = (numerator ^ denominator) & 0x80000000;
	int result = __aeabi_uidivmod((numerator < 0) ? -numerator : numerator,
			(denominator < 0) ? -denominator : denominator);
	return neg_result ? -result : result;
}
unsigned __aeabi_uidiv(unsigned numerator, unsigned denominator)
{
	return __aeabi_uidivmod(numerator, denominator);
}

}
//extern "C"

#endif//__GCCE__

#endif//_SYMBIAN
