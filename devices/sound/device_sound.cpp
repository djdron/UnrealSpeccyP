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

#include "../../std.h"
#include "device_sound.h"

//=============================================================================
//	eDeviceSound::eDeviceSound
//-----------------------------------------------------------------------------
eDeviceSound::eDeviceSound() : mix_l(0), mix_r(0)
{
	SetTimings(SNDR_DEFAULT_SYSTICK_RATE, SNDR_DEFAULT_SAMPLE_RATE);
}

const dword TICK_FF = 6;            // oversampling ratio: 2^6 = 64
const dword TICK_F = (1<<TICK_FF);
//const dword MULT_C = 12;   // fixed point precision for 'system tick -> sound tick'
// b = 1+ln2(max_sndtick) = 1+ln2((max_sndfq*TICK_F)/min_intfq) = 1+ln2(48000*64/10) ~= 19.2;
// assert(b+MULT_C <= 32)

//=============================================================================
//	eDeviceSound::FrameStart
//-----------------------------------------------------------------------------
void eDeviceSound::FrameStart(dword tacts)
{
	dword endtick = (tacts * (qword)sample_rate * TICK_F) / clock_rate; //prev frame rest
	base_tick = tick - endtick;
}
//=============================================================================
//	eDeviceSound::Update
//-----------------------------------------------------------------------------
void eDeviceSound::Update(dword tact, dword l, dword r)
{
	if(!((l ^ mix_l) | (r ^ mix_r)))
		return;
	dword endtick = (tact * (qword)sample_rate * TICK_F) / clock_rate;
	Flush(base_tick + endtick);
	mix_l = l; mix_r = r;
}
//=============================================================================
//	eDeviceSound::FrameEnd
//-----------------------------------------------------------------------------
void eDeviceSound::FrameEnd(dword tacts)
{
	dword endtick = (tacts * (qword)sample_rate * TICK_F) / clock_rate;
	Flush(base_tick + endtick);
}
//=============================================================================
//	eDeviceSound::AudioData
//-----------------------------------------------------------------------------
void* eDeviceSound::AudioData()
{
	return buffer;
}
//=============================================================================
//	eDeviceSound::AudioDataReady
//-----------------------------------------------------------------------------
dword eDeviceSound::AudioDataReady()
{
	return (dstpos - buffer)*sizeof(SNDSAMPLE);
}
//=============================================================================
//	eDeviceSound::AudioDataUse
//-----------------------------------------------------------------------------
void eDeviceSound::AudioDataUse(dword size)
{
	assert(size == AudioDataReady());
	dstpos = buffer;
}
//=============================================================================
//	eDeviceSound::SetTimings
//-----------------------------------------------------------------------------
void eDeviceSound::SetTimings(dword _clock_rate, dword _sample_rate)
{
	clock_rate = _clock_rate;
	sample_rate = _sample_rate;

	tick = base_tick = 0;
	dstpos = buffer;
}

static dword filter_diff[TICK_F*2];
const double filter_sum_full = 1.0, filter_sum_half = 0.5;
const dword filter_sum_full_u = (dword)(filter_sum_full * 0x10000);
const dword filter_sum_half_u = (dword)(filter_sum_half * 0x10000);

//=============================================================================
//	eDeviceSound::Flush
//-----------------------------------------------------------------------------
void eDeviceSound::Flush(dword endtick)
{
	dword scale;
	if(!((endtick ^ tick) & ~(TICK_F-1)))
	{
		//same discrete as before
		scale = filter_diff[(endtick & (TICK_F-1)) + TICK_F] - filter_diff[(tick & (TICK_F-1)) + TICK_F];
		s2_l += mix_l * scale;
		s2_r += mix_r * scale;

		scale = filter_diff[endtick & (TICK_F-1)] - filter_diff[tick & (TICK_F-1)];
		s1_l += mix_l * scale;
		s1_r += mix_r * scale;

		tick = endtick;
	}
	else
	{
		scale = filter_sum_full_u - filter_diff[(tick & (TICK_F-1)) + TICK_F];

		dword sample_value;
		sample_value =	((mix_l*scale + s2_l) >> 16) +
						((mix_r*scale + s2_r) & 0xFFFF0000);

		dstpos->sample = sample_value;
		dstpos++;
		if(dstpos - buffer >= BUFFER_LEN)
		{
			dstpos = buffer;
		}

		scale = filter_sum_half_u - filter_diff[tick & (TICK_F-1)];
		s2_l = s1_l + mix_l * scale;
		s2_r = s1_r + mix_r * scale;

		tick = (tick | (TICK_F-1))+1;

		if((endtick ^ tick) & ~(TICK_F-1))
		{
			// assume filter_coeff is symmetric
			dword val_l = mix_l * filter_sum_half_u;
			dword val_r = mix_r * filter_sum_half_u;
			do
			{
				dword sample_value;
				sample_value =	((s2_l + val_l) >> 16) +
								((s2_r + val_r) & 0xFFFF0000); // save s2+val

				dstpos->sample = sample_value;
				dstpos++;
				if(dstpos - buffer >= BUFFER_LEN)
				{
					dstpos = buffer;
				}

				tick += TICK_F;
				s2_l = val_l;
				s2_r = val_r; // s2=s1, s1=0;

			} while ((endtick ^ tick) & ~(TICK_F-1));
		}

		tick = endtick;

		scale = filter_diff[(endtick & (TICK_F-1)) + TICK_F] - filter_sum_half_u;
		s2_l += mix_l * scale;
		s2_r += mix_r * scale;

		scale = filter_diff[endtick & (TICK_F-1)];
		s1_l = mix_l * scale;
		s1_r = mix_r * scale;
	}
}

const double filter_coeff[TICK_F*2] =
{
	// filter designed with Matlab's DSP toolbox
	0.000797243121022152, 0.000815206499600866, 0.000844792477531490, 0.000886460636664257,
	0.000940630171246217, 0.001007677515787512, 0.001087934129054332, 0.001181684445143001,
	0.001289164001921830, 0.001410557756409498, 0.001545998595893740, 0.001695566052785407,
	0.001859285230354019, 0.002037125945605404, 0.002229002094643918, 0.002434771244914945,
	0.002654234457752337, 0.002887136343664226, 0.003133165351783907, 0.003391954293894633,
	0.003663081102412781, 0.003946069820687711, 0.004240391822953223, 0.004545467260249598,
	0.004860666727631453, 0.005185313146989532, 0.005518683858848785, 0.005860012915564928,
	0.006208493567431684, 0.006563280932335042, 0.006923494838753613, 0.007288222831108771,
	0.007656523325719262, 0.008027428904915214, 0.008399949736219575, 0.008773077102914008,
	0.009145787031773989, 0.009517044003286715, 0.009885804729257883, 0.010251021982371376,
	0.010611648461991030, 0.010966640680287394, 0.011314962852635887, 0.011655590776166550,
	0.011987515680350414, 0.012309748033583185, 0.012621321289873522, 0.012921295559959939,
	0.013208761191466523, 0.013482842243062109, 0.013742699838008606, 0.013987535382970279,
	0.014216593638504731, 0.014429165628265581, 0.014624591374614174, 0.014802262449059521,
	0.014961624326719471, 0.015102178534818147, 0.015223484586101132, 0.015325161688957322,
	0.015406890226980602, 0.015468413001680802, 0.015509536233058410, 0.015530130313785910,
	0.015530130313785910, 0.015509536233058410, 0.015468413001680802, 0.015406890226980602,
	0.015325161688957322, 0.015223484586101132, 0.015102178534818147, 0.014961624326719471,
	0.014802262449059521, 0.014624591374614174, 0.014429165628265581, 0.014216593638504731,
	0.013987535382970279, 0.013742699838008606, 0.013482842243062109, 0.013208761191466523,
	0.012921295559959939, 0.012621321289873522, 0.012309748033583185, 0.011987515680350414,
	0.011655590776166550, 0.011314962852635887, 0.010966640680287394, 0.010611648461991030,
	0.010251021982371376, 0.009885804729257883, 0.009517044003286715, 0.009145787031773989,
	0.008773077102914008, 0.008399949736219575, 0.008027428904915214, 0.007656523325719262,
	0.007288222831108771, 0.006923494838753613, 0.006563280932335042, 0.006208493567431684,
	0.005860012915564928, 0.005518683858848785, 0.005185313146989532, 0.004860666727631453,
	0.004545467260249598, 0.004240391822953223, 0.003946069820687711, 0.003663081102412781,
	0.003391954293894633, 0.003133165351783907, 0.002887136343664226, 0.002654234457752337,
	0.002434771244914945, 0.002229002094643918, 0.002037125945605404, 0.001859285230354019,
	0.001695566052785407, 0.001545998595893740, 0.001410557756409498, 0.001289164001921830,
	0.001181684445143001, 0.001087934129054332, 0.001007677515787512, 0.000940630171246217,
	0.000886460636664257, 0.000844792477531490, 0.000815206499600866, 0.000797243121022152
};

//=============================================================================
//	eFilterDiffInit
//-----------------------------------------------------------------------------
static struct eFilterDiffInit
{
	eFilterDiffInit()
	{
		double sum = 0;
		for(int i = 0; i < (int)TICK_F*2; i++)
		{
			filter_diff[i] = (int)(sum * 0x10000);
			sum += filter_coeff[i];
		}
	}
} fdi;
