void inc8(byte& x)
{
	f = incf[x] | (f & CF);
	x++;
}
void dec8(byte& x)
{
	f = decf[x] | (f & CF);
	x--;
}
void add8(byte src)
{
	f = adcf[a + src*0x100];
	a += src;
}
void adc8(byte src)
{
	byte carry = f & CF;
	f = adcf[a + src*0x100 + 0x10000*carry];
	a += src + carry;
}
void sub8(byte src)
{
	f = sbcf[a*0x100 + src];
	a -= src;
}
void sbc8(byte src)
{
	byte carry = f & CF;
	f = sbcf[a*0x100 + src + 0x10000*carry];
	a -= src + carry;
}
void and8(byte src)
{
	a &= src;
	f = log_f[a] | HF;
}
void or8(byte src)
{
	a |= src;
	f = log_f[a];
}
void xor8(byte src)
{
	a ^= src;
	f = log_f[a];
}
void cp8(byte src)
{
	f = cpf[a*0x100 + src];
}
void bit(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF) | (src & (F3|F5));
}
void bitmem(byte src, byte bit)
{
	f = log_f[src & (1 << bit)] | HF | (f & CF);
	f = (f & ~(F3|F5)) | (mem_h & (F3|F5));
}
void res(byte& src, byte bit) const
{
	src &= ~(1 << bit);
}
byte resbyte(byte src, byte bit) const
{
	return src & ~(1 << bit);
}
void set(byte& src, byte bit) const
{
	src |= (1 << bit);
}
byte setbyte(byte src, byte bit) const
{
	return src | (1 << bit);
}
