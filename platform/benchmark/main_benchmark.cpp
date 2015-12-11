/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2013 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

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

#include "../platform.h"
#include "../../tools/tick.h"

#ifdef USE_BENCHMARK

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		printf("Usage : %s image_name\n", argv[0]);
		return 1;
	}
	int r = 0;
	using namespace xPlatform;
	Handler()->OnInit();
	const int benchmark_real_time = 600;
	if(Handler()->OnOpenFile(argv[1]))
	{
		printf("Emulating %d real sec. (%d frames)...", benchmark_real_time, benchmark_real_time*50);
		fflush(stdout);
		eTick tick_start;
		tick_start.SetCurrent();
		for(int f = benchmark_real_time*50; --f >= 0;)
		{
			Handler()->OnLoop();
		}
		float t = tick_start.Passed().Sec();
		printf("done in %g sec. (%g:1 ratio)\n", t, float(benchmark_real_time)/t);
	}
	else
	{
		printf("Error : %s - unsupported image format\n", argv[1]);
		r = 1;
	}
	Handler()->OnDone();
	return r;
}

#endif//USE_BENCHMARK
