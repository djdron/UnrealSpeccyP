#ifndef __EMUL2203_H__
#define __EMUL2203_H__

#include "../std_types.h"

typedef short stream_sample_t;
typedef stream_sample_t FMSAMPLE;

/* select bit size of output : 8 or 16 */
#define FM_SAMPLE_BITS 16

/* select timer system internal or external */
#define FM_INTERNAL_TIMER 0

/* --- speedup optimize --- */
/* busy flag emulation , The definition of FM_GET_TIME_NOW() is necessary. */
#define FM_BUSY_FLAG_SUPPORT 1


/* struct describing a single operator (SLOT) */
typedef struct
{
	int	*DT;		/* detune          :dt_tab[DT] */
	byte	KSR;		/* key scale rate  :3-KSR */
	dword	ar;		/* attack rate  */
	dword	d1r;		/* decay rate   */
	dword	d2r;		/* sustain rate */
	dword	rr;		/* release rate */
	byte	ksr;		/* key scale rate  :kcode>>(3-KSR) */
	dword	mul;		/* multiple        :ML_TABLE[ML] */

	/* Phase Generator */
	dword	phase;		/* phase counter */
	dword	Incr;		/* phase step */

	/* Envelope Generator */
	byte	state;		/* phase type */
	dword	tl;		/* total level: TL << 3 */
	int	volume;		/* envelope counter */
	dword	sl;		/* sustain level:sl_table[SL] */
	dword	vol_out;	/* current output from EG circuit (without AM from LFO) */

	byte	eg_sh_ar;	/*  (attack state) */
	byte	eg_sel_ar;	/*  (attack state) */
	byte	eg_sh_d1r;	/*  (decay state) */
	byte	eg_sel_d1r;	/*  (decay state) */
	byte	eg_sh_d2r;	/*  (sustain state) */
	byte	eg_sel_d2r;	/*  (sustain state) */
	byte	eg_sh_rr;	/*  (release state) */
	byte	eg_sel_rr;	/*  (release state) */

	byte	ssg;		/* SSG-EG waveform */
	byte	ssgn;		/* SSG-EG negated output */

	dword	key;		/* 0=last key was KEY OFF, 1=KEY ON */

} FM_SLOT;

typedef struct
{
	FM_SLOT	SLOT[4];	/* four SLOTs (operators) */

	byte	ALGO;		/* algorithm */
	byte	FB;		/* feedback shift */
	int	op1_out[2];	/* op1 output for feedback */

	int	*connect1;	/* SLOT1 output pointer */
	int	*connect2;	/* SLOT2 output pointer */
	int	*connect3;	/* SLOT3 output pointer */
	int	*connect4;	/* SLOT4 output pointer */

	int	*mem_connect;/* where to put the delayed sample (MEM) */
	int	mem_value;	/* delayed sample (MEM) value */

	int	pms;		/* channel PMS */
	byte	ams;		/* channel AMS */

	dword	fc;		/* fnum,blk:adjusted to sample rate */
	byte	kcode;		/* key code:                        */
	dword	block_fnum;	/* current blk/fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
} FM_CH;

typedef struct
{
	void *	param;		/* this chip parameter  */
	int	clock;		/* master clock  (Hz)   */
	int	SSGclock;	/* clock for SSG (Hz)   */
	int	rate;		/* sampling rate (Hz)   */
	double	freqbase;	/* frequency base       */
	double	TimerBase;	/* Timer base time      */
#if FM_BUSY_FLAG_SUPPORT
	double	BusyExpire;	/* ExpireTime of Busy clear */
#endif
	byte	address;	/* address register     */
	byte	irq;		/* interrupt level      */
	byte	irqmask;	/* irq mask             */
	byte	status;		/* status flag          */
	dword	mode;		/* mode  CSM / 3SLOT    */
	byte	prescaler_sel;/* prescaler selector */
	byte	fn_h;		/* freq latch           */
	int	TA;		/* timer a              */
	int	TAC;		/* timer a counter      */
	byte	TB;		/* timer b              */
	int	TBC;		/* timer b counter      */
	/* local time tables */
	int	dt_tab[8][32];/* DeTune table       */
	/* Extention Timer and IRQ handler */
//	FM_TIMERHANDLER	Timer_Handler;
//	FM_IRQHANDLER	IRQ_Handler;
//	const struct ssg_callbacks *SSG;
} FM_ST;

/***********************************************************/
/* OPN unit                                                */
/***********************************************************/

/* OPN 3slot struct */
typedef struct
{
	dword  fc[3];			/* fnum3,blk3: calculated */
	byte	fn_h;			/* freq3 latch */
	byte	kcode[3];		/* key code */
	dword	block_fnum[3];	/* current fnum value for this slot (can be different betweeen slots of one channel in 3slot mode) */
} FM_3SLOT;

/* OPN/A/B common state */
typedef struct
{
	byte	type;			/* chip type */
	FM_ST	ST;			/* general state */
	FM_3SLOT SL3;			/* 3 slot mode state */
	FM_CH	*P_CH;			/* pointer of CH */

	dword	eg_cnt;			/* global envelope generator counter */
	dword	eg_timer;		/* global envelope generator counter works at frequency = chipclock/64/3 */
	dword	eg_timer_add;	/* step of eg_timer */
	dword	eg_timer_overflow;/* envelope generator timer overlfows every 3 samples (on real chip) */


	/* there are 2048 FNUMs that can be generated using FNUM/BLK registers
        but LFO works with one more bit of a precision so we really need 4096 elements */

	dword	fn_table[2048];	/* fnumber->increment counter */

} FM_OPN;

/* here's the virtual YM2203(OPN) */
typedef struct
{
	byte REGS[256];		/* registers         */
	FM_OPN OPN;				/* OPN state         */
	FM_CH CH[3];			/* channel state     */
} YM2203;

#define FM_GET_TIME_NOW() 0
//timer_get_time()

#ifndef PI
#define PI 3.14159265358979323846
#endif

/* -------------------- YM2203(OPN) Interface -------------------- */

/*
** Initialize YM2203 emulator(s).
**
** 'num'           is the number of virtual YM2203's to allocate
** 'baseclock'
** 'rate'          is sampling rate
** 'TimerHandler'  timer callback handler when timer start and clear
** 'IRQHandler'    IRQ callback handler when changed IRQ level
** return      0 = success
*/
void * YM2203Init(void *param, int index, int baseclock, int rate
//			,FM_TIMERHANDLER TimerHandler,FM_IRQHANDLER IRQHandler, const struct ssg_callbacks *ssg
			   );

/*
** shutdown the YM2203 emulators
*/
void YM2203Shutdown(void *chip);

/*
** reset all chip registers for YM2203 number 'num'
*/
void YM2203ResetChip(void *chip);

/*
** update one of chip
*/
void YM2203UpdateOne(void *chip, FMSAMPLE *buffer, int length);

/*
** Write
** return : InterruptLevel
*/
int YM2203Write(void *chip,int a,unsigned char v);

/*
** Read
** return : InterruptLevel
*/
unsigned char YM2203Read(void *chip,int a);

/*
**  Timer OverFlow
*/
int YM2203TimerOver(void *chip, int c);

/*
**  State Save
*/
//void YM2203Postload(void *chip);

//void YM2203_save_state(void *chip, int index);

#endif//__EMUL2203_H__
