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
