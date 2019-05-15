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

#define LCDD0_H	GpioDataRegs.GPBSET.bit.GPIO57 = 1
#define LCDD1_H	GpioDataRegs.GPBSET.bit.GPIO56 = 1
#define LCDD2_H	GpioDataRegs.GPBSET.bit.GPIO61 = 1
#define LCDD3_H	GpioDataRegs.GPBSET.bit.GPIO60 = 1
#define LCDE_H	GpioDataRegs.GPBSET.bit.GPIO55 = 1
#define LCDRS_H	GpioDataRegs.GPBSET.bit.GPIO54 = 1
#define LCDD0_L	GpioDataRegs.GPBCLEAR.bit.GPIO57 = 1
#define LCDD1_L	GpioDataRegs.GPBCLEAR.bit.GPIO56 = 1
#define LCDD2_L	GpioDataRegs.GPBCLEAR.bit.GPIO61 = 1
#define LCDD3_L	GpioDataRegs.GPBCLEAR.bit.GPIO60 = 1
#define LCDE_L	GpioDataRegs.GPBCLEAR.bit.GPIO55 = 1
#define LCDRS_L	GpioDataRegs.GPBCLEAR.bit.GPIO54 = 1

#define LINE1 0x80
#define LINE2 0x40
#define CHARACTER_NUMBER_MAX 16

interrupt void XINT1_isr(void);
interrupt void XINT2_isr(void);
interrupt void cpu_timer0_isr(void);
interrupt void cpu_timer1_isr(void);
interrupt void cpu_timer2_isr(void);

void Gpio_select(void);
void lcdprint_data(char *str);
void lcd_write(char data,unsigned char Rs);
void lcd_Gpio_data_out(unsigned char da);
void lcd_init(void);

inline void IncreaseTick(unsigned int tick);
void ShowTime();

enum TimerMode{
	Timer,
	StopWatch
};

unsigned int Tick = 0;
unsigned int SavedTick = 0;
unsigned int ClickedNumber = 0;
unsigned int Trigger = 0;
int Mode = Timer;

interrupt void XINT1_isr(void)
{
	DINT;

	if(ClickedNumber++ > 0)
	{
		ClickedNumber = 0;
		Tick = 0;
		SavedTick = 0;

		lcd_write(LINE1 + LINE2 + 1, 0); // Move Cursor
		lcdprint_data("Initialized!");
		Trigger = 1;
	}
	SavedTick = Tick;
	LED1_T;

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void XINT2_isr(void)
{
	DINT;

	ClickedNumber = 0;
	if(Mode == Timer) Mode = StopWatch;
	else if(Mode == StopWatch) Mode = Timer;
	LED2_T;

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void cpu_timer0_isr(void)
{
	DINT;

	IncreaseTick(1);
	if(Mode == Timer) ShowTime();
	else if(Mode == StopWatch) ShowSavedTime();

	if(Trigger && Tick >= 30)
	{
		Trigger = 0;
		lcd_write(LINE1 + LINE2 + 1, 0); // Move Cursor
		lcdprint_data("            "); // Erase
	}

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; // Reset bit for re-interrupt
}

void main(void)
{
	InitSysCtrl();

	Gpio_select();

	DINT;

	lcd_init(); // LCD Initialize
	InitPieCtrl();

	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable();

	EALLOW;
	PieVectTable.XINT1 = &XINT1_isr;
	PieVectTable.XINT2 = &XINT2_isr;

	PieVectTable.TINT0 = &cpu_timer0_isr;

	GpioCtrlRegs.GPAMUX2.bit.GPIO26 = 0;
	GpioCtrlRegs.GPAMUX2.bit.GPIO27 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO26 = 0;
	GpioCtrlRegs.GPADIR.bit.GPIO27 = 0;

	GpioIntRegs.GPIOXINT1SEL.bit.GPIOSEL=27;
	GpioIntRegs.GPIOXINT2SEL.bit.GPIOSEL=26;
	EDIS;

	XIntruptRegs.XINT1CR.bit.POLARITY = 0;
	XIntruptRegs.XINT2CR.bit.POLARITY = 0;
	XIntruptRegs.XINT1CR.bit.ENABLE = 1;
	XIntruptRegs.XINT2CR.bit.ENABLE = 1;

	// Interrupt Vector Inserting
	InitCpuTimers();

	// Timer Cycle Setting
	ConfigCpuTimer(&CpuTimer0, 15000000, 1); // 10Hz Clock (0.1s)

	PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // get vector from PIE vector table except for 'Reset'
	PieCtrlRegs.PIEIER1.bit.INTx4 = 1;
	PieCtrlRegs.PIEIER1.bit.INTx5 = 1;
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
	IER |= M_INT1 | M_INT13 | M_INT14;
	EINT;

	LED1_L;
	LED2_L;

	for(;;);

}

void Gpio_select(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX2.all = 0x0000;
	GpioCtrlRegs.GPBMUX2.all = 0x0000;

	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;

	// LCD Register
	GpioCtrlRegs.GPBDIR.bit.GPIO54 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO55 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO57 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO56 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO61 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO60 = 1;

	EDIS;
}

void lcdprint_data(char *str)
{
	unsigned char i=0;
	while(str[i]!='\0')
	{
		lcd_write(str[i++],1);
	}
}

void lcd_write(char data,unsigned char Rs)
{
	if(Rs)	LCDRS_H;
	else	LCDRS_L;
	lcd_Gpio_data_out((data>>4) & 0x0f);
	DELAY_US(1);
	LCDE_H;
	DELAY_US(1);
	LCDE_L;
	DELAY_US(1);

	lcd_Gpio_data_out(data & 0x0f);
	DELAY_US(1);
	LCDE_H;
	DELAY_US(1);
	LCDE_L;
	DELAY_US(41);
}

void lcd_Gpio_data_out(unsigned char da)
{
	if(da & 0x1)	LCDD0_H;
	else	LCDD0_L;
	if(da & 0x2)	LCDD1_H;
	else	LCDD1_L;
	if(da & 0x4)	LCDD2_H;
	else	LCDD2_L;
	if(da & 0x8)	LCDD3_H;
	else	LCDD3_L;
}

void lcd_init(void)
{
	lcd_write(0x28,0);		//4bit data mode, 2 line, 5x7 dot
	lcd_write(0x28,0);
	lcd_write(0x0C,0);		//display on
	lcd_write(0x01,0);		//Display Clear
    DELAY_US(1960);
	lcd_write(0x06,0);		//Entry mode
}

inline void IncreaseTick(unsigned int tick)
{
	Tick += tick;
}

//=================================== Custom Function =========================================

void ShowTime()
{
	lcd_write(LINE1 + 1, 0); // Move Cursor
	int millisecond = Tick % 10; // 0.1s
	int second = (int)(Tick / 10) % 60; // 1s
	int minute = (int)(Tick / 600) % 60; // 1m

	// minute
	lcd_write((char)(minute / 10 % 10 + 48), 1);
	lcd_write((char)(minute % 10 + 48), 1);
	lcd_write(':', 1);

	// second
	lcd_write((char)(second / 10 % 10 + 48), 1);
	lcd_write((char)(second % 10 + 48), 1);
	lcd_write(':', 1);

	// milli second
	lcd_write((char)(millisecond + 48), 1);
}

void ShowSavedTime()
{
	lcd_write(LINE1 + 1, 0); // Move Cursor
	int millisecond = SavedTick % 10; // 0.1s
	int second = (int)(SavedTick / 10) % 60; // 1s
	int minute = (int)(SavedTick / 600) % 60; // 1m

	// minute
	lcd_write((char)(minute / 10 % 10 + 48), 1);
	lcd_write((char)(minute % 10 + 48), 1);
	lcd_write(':', 1);

	// second
	lcd_write((char)(second / 10 % 10 + 48), 1);
	lcd_write((char)(second % 10 + 48), 1);
	lcd_write(':', 1);

	// milli second
	lcd_write((char)(millisecond + 48), 1);
}
