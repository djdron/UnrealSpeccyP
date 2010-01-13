#include "std.h"

#include "speccy.h"

int main(int argc, char** argv)
{
	eSpeccy* speccy = new eSpeccy;
	speccy->Init();
	speccy->Reset();
	while(1)
	{
		speccy->Update();
	}
	delete(speccy);
	return 0;
}
