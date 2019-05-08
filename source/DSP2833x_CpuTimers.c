// TI File $Revision: /main/4 $
// Checkin $Date: July 9, 2009   10:51:59 $
//###########################################################################
//
// FILE:    DSP2833x_CpuTimers.c
//
// TITLE:   CPU 32-bit Timers Initialization & Support Functions.
//
// NOTES:   CpuTimer2 is reserved for use with DSP BIOS and
//          other realtime operating systems.
//
//          Do not use these this timer in your application if you ever plan
//          on integrating DSP-BIOS or another realtime OS.
//
//###########################################################################
// $TI Release: DSP2833x/DSP2823x C/C++ Header Files V1.31 $
// $Release Date: August 4, 2009 $
//###########################################################################

#include "DSP2833x_Device.h"     // Headerfile Include File
#include "DSP2833x_Examples.h"   // Examples Include File

struct CPUTIMER_VARS CpuTimer0;

// When using DSP BIOS & other RTOS, comment out CPU Timer 2 code.
struct CPUTIMER_VARS CpuTimer1;
struct CPUTIMER_VARS CpuTimer2;

//---------------------------------------------------------------------------
// InitCpuTimers:
//---------------------------------------------------------------------------
// This function initializes all three CPU timers to a known state.
//
void InitCpuTimers(void)
{
	CpuTimer0.RegsAddr = &CpuTimer0Regs;
	CpuTimer1.RegsAddr = &CpuTimer1Regs;
	CpuTimer2.RegsAddr = &CpuTimer2Regs;
}

void ConfigCpuTimer(struct CPUTIMER_VARS *Timer, Uint32 Timer_Counter, Uint16 tddr)
{
	Timer->RegsAddr->PRD.all = Timer_Counter;

	if(tddr>0)		tddr--;

	Timer->RegsAddr->TPR.all  = tddr & 0xff;
	Timer->RegsAddr->TPRH.all  = (tddr>>8) & 0xff;

	Timer->RegsAddr->TCR.bit.TSS = 1;
	Timer->RegsAddr->TCR.bit.TRB = 1;
	Timer->RegsAddr->TCR.bit.SOFT = 1;
	Timer->RegsAddr->TCR.bit.FREE = 1;
	Timer->RegsAddr->TCR.bit.TIE = 1;

	Timer->InterruptCount = 0;

	Timer->RegsAddr->TCR.bit.TSS = 0;
}
