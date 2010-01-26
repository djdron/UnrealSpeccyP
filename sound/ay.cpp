#include "../std.h"
#include "ay.h"
#include "ym2203emu.h"
#include "../z80.h"

eAY::eAY(xZ80::eZ80* _cpu) : cpu(_cpu)
{
   bitA = bitB = bitC = 0;
   nextfmtick = 0; //Alone Coder
   SetTimings(SNDR_DEFAULT_SYSTICK_RATE, SNDR_DEFAULT_AY_RATE, SNDR_DEFAULT_SAMPLE_RATE);
   chip2203 = YM2203Init(NULL, 0, SNDR_DEFAULT_AY_RATE, 44100);
   SetChip(CHIP_AY);
   SetVolumes(0x7FFF, SNDR_VOL_AY, SNDR_PAN_ABC);
   Reset();
}

eAY::~eAY()
{
	YM2203Shutdown(chip2203);
}

void eAY::IoRead(word port, byte* v)
{
	if((byte)port == 0xFD)
	{
		if((port & 0xC0FF) != 0xC0FD)
			*v = 0xFF;
		*v = Read();
	}
}

void eAY::IoWrite(word port, byte v)
{
   if(port & 2)
	   return;
   if((port & 0xC0FF) == 0xC0FD)
   {
       Select(v);
   }
   if((port & 0xC000) == 0x8000)
   {
      Write(cpu->T(), v);
   }
}

dword eAY::Render(AYOUT *src, dword srclen, dword clk_ticks, bufptr_t dst)
{
   StartFrame(dst);
   for (dword index = 0; index < srclen; index++) {
      // if (src[index].timestamp > clk_ticks) continue; // wrong input data leads to crash
      Select(src[index].reg_num);
      Write(src[index].timestamp, src[index].reg_value);
   }
   return EndFrame(clk_ticks);
}


const dword MULT_C_1 = 14; // fixed point precision for 'system tick -> ay tick'
// b = 1+ln2(max_ay_tick/8) = 1+ln2(max_ay_fq/8 / min_intfq) = 1+ln2(10000000/(10*8)) = 17.9
// assert(b+MULT_C_1 <= 32)

void eAY::StartFrame(bufptr_t dst)
{
   r13_reloaded = 0;
   eInherited::StartFrame(dst);
}

dword eAY::EndFrame(dword clk_ticks)
{
   // adjusting 't' with whole history will fix accumulation of rounding errors

   qword end_chip_tick = ((passed_clk_ticks + clk_ticks) * chip_clock_rate) / system_clock_rate;

   Flush( (dword) (end_chip_tick - passed_chip_ticks) );
   dword res = eInherited::EndFrame(t);

   passed_clk_ticks += clk_ticks;
   passed_chip_ticks += t; t = 0;
   nextfmtickfloat = 0.; //Alone Coder
   nextfmtick = 0; //Alone Coder

   return res;
}

void eAY::Flush(dword chiptick) // todo: noaction at (temp.sndblock || !conf.sound.ay)
{
   while (t < chiptick) {
      t++;
      if (++ta >= fa) ta = 0, bitA ^= -1;
      if (++tb >= fb) tb = 0, bitB ^= -1;
      if (++tc >= fc) tc = 0, bitC ^= -1;
      if (++tn >= fn)
         tn = 0,
         ns = (ns*2+1) ^ (((ns>>16)^(ns>>13)) & 1),
         bitN = 0 - ((ns >> 16) & 1);
      if (++te >= fe) {
         te = 0, env += denv;
         if (env & ~31) {
            dword mask = (1<<r.env);
            if (mask & ((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<15)))
               env = denv = 0;
            else if (mask & ((1<<8)|(1<<12)))
               env &= 31;
            else if (mask & ((1<<10)|(1<<14)))
               denv = -denv, env = env + denv;
            else env = 31, denv = 0; //11,13
         }
      }

      dword en, mix_l, mix_r;

      en = ((ea & env) | va) & ((bitA | bit0) & (bitN | bit3));
      mix_l  = vols[0][en]; mix_r  = vols[1][en];

      en = ((eb & env) | vb) & ((bitB | bit1) & (bitN | bit4));
      mix_l += vols[2][en]; mix_r += vols[3][en];

      en = ((ec & env) | vc) & ((bitC | bit2) & (bitN | bit5));
      mix_l += vols[4][en]; mix_r += vols[5][en];
//YM2203 here
/*      if(conf.sound.ay_chip == CHIP_YM2203)
      {
        if (t >= nextfmtick) {
          nextfmtickfloat += ayticks_per_fmtick;
		  nextfmtick = (int)nextfmtickfloat;
		  if (++FMbufN == FMBUFSIZE) {
            YM2203UpdateOne(chip2203, FMbufs, FMBUFSIZE);
			FMbufN = 0;
		  };
          if (fmsoundon0 == 0) {
            //FMbufOUT=(int)(FMbuf*conf.sound.ay/8192*0.7f);
		    FMbufOUT=((((INT16)FMbufs[FMbufN])*FMbufMUL)>>16);
		  }
		  else FMbufOUT=0;
		}
        mix_l += FMbufOUT; mix_r += FMbufOUT;
      }; //Alone Coder
*/
//
      if ((mix_l ^ eInherited::mix_l) | (mix_r ^ eInherited::mix_r)) // similar check inside update()
         Update(t, mix_l, mix_r);
   }
}

void eAY::Select(byte nreg)
{
   if (chiptype == CHIP_AY) nreg &= 0x0F;
   activereg = nreg;
}

void eAY::Write(dword timestamp, byte val)
{
/*
   if(activereg >= 0x20 && conf.sound.ay_chip == CHIP_YM2203)
   {
      if (timestamp) Flush((timestamp * mult_const) >> MULT_C_1); // cputick * ( (chip_clock_rate/8) / system_clock_rate );
      if (activereg >= 0x2d && activereg <= 0x2f) {
         int oldayfq=chip2203->OPN.ST.SSGclock;//ayfq
         YM2203Write(chip2203,0,activereg);
         YM2203Write(chip2203,1,val);
         if (oldayfq!=chip2203->OPN.ST.SSGclock) {
            //if (!conf.sound.ay_samples) Flush(cpu.t);
            //ayfq=chip2203->OPN.ST.SSGclock;
            //t=(dword)((__int64)t*ayfq/oldayfq);
            //mult_const2 = ((ayfq/conf.intfq) << (MULT_C_1-3))/conf.frame;
            //mult_const3 = TICK_F/2+(dword)((__int64)temp.snd_frame_ticks*conf.intfq*(1<<(MULT_C+3))/ayfq);
            //ay_div = ((dword)((double)ayfq*0x10*(double)SAMPLE_T/(double)conf.sound.fq));
            //ay_div2 = (ayfq*0x100)/(conf.sound.fq/32);
			SetTimings(system_clock_rate,chip2203->OPN.ST.SSGclock,eInherited::sample_rate);
         }
      }
      else
      {
         YM2203Write(chip2203,0,activereg);
         YM2203Write(chip2203,1,val);
      }
      return;
   } //Dexus
*/
   if (activereg >= 0x10) return;

   if ((1 << activereg) & ((1<<1)|(1<<3)|(1<<5)|(1<<13))) val &= 0x0F;
   if ((1 << activereg) & ((1<<6)|(1<<8)|(1<<9)|(1<<10))) val &= 0x1F;

   if (activereg != 13 && reg[activereg] == val) return;

   reg[activereg] = val;

   if (timestamp) Flush((timestamp * mult_const) >> MULT_C_1); // cputick * ( (chip_clock_rate/8) / system_clock_rate );

   switch(activereg)
   {
      case 0:
      case 1:
         fa = r.fA;
         break;
      case 2:
      case 3:
         fb = r.fB;
         break;
      case 4:
      case 5:
         fc = r.fC;
         break;
      case 6:
         fn = val*2;
         break;
      case 7:
         bit0 = 0 - ((val>>0) & 1);
         bit1 = 0 - ((val>>1) & 1);
         bit2 = 0 - ((val>>2) & 1);
         bit3 = 0 - ((val>>3) & 1);
         bit4 = 0 - ((val>>4) & 1);
         bit5 = 0 - ((val>>5) & 1);
         break;
      case 8:
         ea = (val & 0x10)? -1 : 0;
         va = ((val & 0x0F)*2+1) & ~ea;
         break;
      case 9:
         eb = (val & 0x10)? -1 : 0;
         vb = ((val & 0x0F)*2+1) & ~eb;
         break;
      case 10:
         ec = (val & 0x10)? -1 : 0;
         vc = ((val & 0x0F)*2+1) & ~ec;
         break;
      case 11:
      case 12:
         fe = r.envT;
         break;
      case 13:
         r13_reloaded = 1;
         te = 0;
         if (r.env & 4) env = 0, denv = 1; // attack
         else env = 31, denv = -1; // decay
         break;
   }
}

byte eAY::Read()
{
   if(activereg >= 0x10)
	   return 0xFF;
   return reg[activereg & 0x0F];
}

void eAY::SetTimings(dword system_clock_rate, dword chip_clock_rate, dword sample_rate)
{
/*
   if(conf.sound.ay_chip == CHIP_YM2203)
   { //install YM2203 frequencies
         chip2203->OPN.ST.clock = conf.sound.ayfq*2;
         chip2203->OPN.ST.rate = 44100
         OPNPrescaler_w(&chip2203->OPN, 1 , 1 );
         //ayfq=chip2203->OPN.ST.SSGclock;
         chip_clock_rate=chip2203->OPN.ST.SSGclock;
   } //Dexus
*/
   chip_clock_rate /= 8;

   eAY::system_clock_rate = system_clock_rate;
   eAY::chip_clock_rate = chip_clock_rate;

   mult_const = (dword) (((qword)chip_clock_rate << MULT_C_1) / system_clock_rate);
   eInherited::SetTimings(chip_clock_rate, sample_rate);
   passed_chip_ticks = passed_clk_ticks = 0;
   t = 0; ns = 0xFFFF;

   nextfmtickfloat = 0.; //Alone Coder
   nextfmtick = 0; //Alone Coder
   ayticks_per_fmtick = (float)chip_clock_rate/44100;
   FMbufMUL=(word)(0.1f*65536); //Alone Coder 0.36.4

   ApplyRegs();
}

void eAY::SetVolumes(dword global_vol, const SNDCHIP_VOLTAB *voltab, const SNDCHIP_PANTAB *stereo)
{
   for (int j = 0; j < 6; j++)
      for (int i = 0; i < 32; i++)
         vols[j][i] = (dword) (((qword)global_vol * voltab->v[i] * stereo->raw[j])/(65535*100*3));
}

void eAY::Reset(dword timestamp)
{
   for (int i = 0; i < 14; i++) reg[i] = 0;

   if (chip2203) YM2203ResetChip(chip2203); //Dexus
/*
   ayfq=chip2203->OPN.ST.SSGclock; //Dexus
   mult_const2 = ((ayfq/conf.intfq) << (MULT_C_1-3))/conf.frame; //Dexus
   mult_const3 = TICK_F/2+(dword)((__int64)temp.snd_frame_ticks*conf.intfq*(1<<(MULT_C+3))/ayfq); //Dexus
   ay_div = ((dword)((double)ayfq*0x10*(double)SAMPLE_T/(double)conf.sound.fq)); //Dexus
   ay_div2 = (ayfq*0x100)/(conf.sound.fq/32); //Dexus
*/
   ApplyRegs(timestamp);
}

void eAY::ApplyRegs(dword timestamp)
{
   for (byte r = 0; r < 16; r++) {
      Select(r); byte p = reg[r];
      /* clr cached values */
      Write(timestamp, p ^ 1);
      Write(timestamp, p);
   }
}

// corresponds enum CHIP_TYPE
const char * const ay_chips[] = { "AY-3-8910", "YM2149F", "YM2203" }; //Dexus

const char* eAY::GetChipName(CHIP_TYPE i) { return ay_chips[i]; }

const SNDCHIP_VOLTAB SNDR_VOL_AY_S =
{ { 0x0000,0x0000,0x0340,0x0340,0x04C0,0x04C0,0x06F2,0x06F2,0x0A44,0x0A44,0x0F13,0x0F13,0x1510,0x1510,0x227E,0x227E,
    0x289F,0x289F,0x414E,0x414E,0x5B21,0x5B21,0x7258,0x7258,0x905E,0x905E,0xB550,0xB550,0xD7A0,0xD7A0,0xFFFF,0xFFFF } };

const SNDCHIP_VOLTAB SNDR_VOL_YM_S =
{ { 0x0000,0x0000,0x00EF,0x01D0,0x0290,0x032A,0x03EE,0x04D2,0x0611,0x0782,0x0912,0x0A36,0x0C31,0x0EB6,0x1130,0x13A0,
    0x1751,0x1BF5,0x20E2,0x2594,0x2CA1,0x357F,0x3E45,0x475E,0x5502,0x6620,0x7730,0x8844,0xA1D2,0xC102,0xE0A2,0xFFFF } };

static const SNDCHIP_PANTAB SNDR_PAN_MONO_S = {{100,100, 100,100, 100,100}};
static const SNDCHIP_PANTAB SNDR_PAN_ABC_S =  {{ 100,10,  66,66,   10,100}};
static const SNDCHIP_PANTAB SNDR_PAN_ACB_S =  {{ 100,10,  10,100,  66,66 }};
static const SNDCHIP_PANTAB SNDR_PAN_BAC_S =  {{ 66,66,   100,10,  10,100}};
static const SNDCHIP_PANTAB SNDR_PAN_BCA_S =  {{ 10,100,  100,10,  66,66 }};
static const SNDCHIP_PANTAB SNDR_PAN_CAB_S =  {{ 66,66,   10,100,  100,10}};
static const SNDCHIP_PANTAB SNDR_PAN_CBA_S =  {{ 10,100,  66,66,   100,10}};

const SNDCHIP_VOLTAB* SNDR_VOL_AY = &SNDR_VOL_AY_S;
const SNDCHIP_VOLTAB* SNDR_VOL_YM = &SNDR_VOL_YM_S;
const SNDCHIP_PANTAB* SNDR_PAN_MONO = &SNDR_PAN_MONO_S;
const SNDCHIP_PANTAB* SNDR_PAN_ABC = &SNDR_PAN_ABC_S;
const SNDCHIP_PANTAB* SNDR_PAN_ACB = &SNDR_PAN_ACB_S;
const SNDCHIP_PANTAB* SNDR_PAN_BAC = &SNDR_PAN_BAC_S;
const SNDCHIP_PANTAB* SNDR_PAN_BCA = &SNDR_PAN_BCA_S;
const SNDCHIP_PANTAB* SNDR_PAN_CAB = &SNDR_PAN_CAB_S;
const SNDCHIP_PANTAB* SNDR_PAN_CBA = &SNDR_PAN_CBA_S;
