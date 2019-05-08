#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define LED1	GpioDataRegs.GPADAT.bit.GPIO25
#define LED2	GpioDataRegs.GPADAT.bit.GPIO24
#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 ÇÉ HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 ÇÉ LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 ÇÉ HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 ÇÉ LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü

#define FNDA_H	GpioDataRegs.GPBSET.bit.GPIO50 = 1
#define FNDB_H	GpioDataRegs.GPBSET.bit.GPIO51 = 1
#define FNDC_H	GpioDataRegs.GPBSET.bit.GPIO52 = 1
#define FNDD_H	GpioDataRegs.GPBSET.bit.GPIO53 = 1
#define FNDA_L	GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1
#define FNDB_L	GpioDataRegs.GPBCLEAR.bit.GPIO51 = 1
#define FNDC_L	GpioDataRegs.GPBCLEAR.bit.GPIO52 = 1
#define FNDD_L	GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1

void Gpio_select(void);
void Gpio_Fnd_out(unsigned char da);

interrupt void cpu_timer0_isr(void)
{
	DINT;

	LED1_T;

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Reset bit for re-interrupt
}

interrupt void cpu_timer1_isr(void)
{
	DINT;

	LED2_T;

	EINT;
}

interrupt void cpu_timer2_isr(void)
{
	static unsigned char counter = 0;

	DINT;

	if(++counter > 9) counter = 0;
	Gpio_Fnd_out(counter);

	EINT;
}



void main(void)
{

	InitSysCtrl();
	Gpio_select();
	DINT;
	InitPieCtrl();

	IER = 0x0000;
	IFR = 0x0000;
	InitPieVectTable();

	// Interrupt Vector Inserting
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	PieVectTable.XINT13 = &cpu_timer1_isr;
	PieVectTable.TINT2 = &cpu_timer2_isr;
	EDIS;

	InitCpuTimers();

	// Timer Cycle Setting
	ConfigCpuTimer(&CpuTimer0, 150000000, 2);
	ConfigCpuTimer(&CpuTimer1, 15000000, 10);
	ConfigCpuTimer(&CpuTimer2, 15000000, 10);

	// Timer Enable
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // get vector from PIE vector table except for 'Reset'
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1; //
	IER |= M_INT1 | M_INT13 | M_INT14;
	EINT;

	LED1_L;
	LED2_L;

	for(;;);

}

void Gpio_select(void)
{
	EALLOW;

	GpioCtrlRegs.GPAMUX2.all = 0; //GPIO16 ~ GPIO31
	GpioCtrlRegs.GPBMUX2.all = 0;

	// LED Register
	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;	//OUTPUT ¼³Á¤
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;	//OUTPUT ¼³Á¤

	// FND Register
	GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;

	EDIS;
}

void Gpio_Fnd_out(unsigned char da)
{
	if(da & 0x01)	FNDA_H;
	else			FNDA_L;
	if(da & 0x02)	FNDB_H;
	else			FNDB_L;
	if(da & 0x04)	FNDC_H;
	else			FNDC_L;
	if(da & 0x08)	FNDD_H;
	else			FNDD_L;
}
