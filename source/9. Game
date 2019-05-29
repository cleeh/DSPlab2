#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "stdlib.h"

#define LED1	GpioDataRegs.GPADAT.bit.GPIO25
#define LED2	GpioDataRegs.GPADAT.bit.GPIO24
#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 ÇÉ HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 ÇÉ LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 ÇÉ HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 ÇÉ LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü

#define DIP1	GpioDataRegs.GPBDAT.bit.GPIO48
#define DIP2	GpioDataRegs.GPBDAT.bit.GPIO49

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

#define GAME_TIME_LIMIT 600 // 300 second

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

typedef enum{
	GNULL,
	GameWin,
	TimeOut
}EGameState;

typedef enum{
	ANULL,
	Success,
	Fail
}EAnswerState;

inline void AddTick(int tick);
inline void ResetTick();
void ShowTime();

int GetRandomNumberBetween(int from, int to);

void InitializeGame();
void EndGame();
void ShowString(char* str);
void ShowGameState();
void ShowAnswerState();
inline int GetUserInput();

int Tick = GAME_TIME_LIMIT;

int TargetNumber = -1;
int UserInput100 = 0;
int UserInput10 = 0;
int UserInput1 = 0;
int UserInputAdder = 1;
int GameOffTrigger = 1;

EGameState GameState = GNULL;

interrupt void XINT1_isr(void)
{
	DINT;

	if(DIP1 && !DIP2) // Start timer & Create random number & Restart game
	{
		InitializeGame();
		ShowAnswer();
	}
	else if(!DIP1 && DIP2) // Game off
	{
		lcd_write(LINE1 + LINE2 + 1, 0); // Move Cursor
		lcdprint_data("Game Off");
		GameOffTrigger = 1;
	}

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void XINT2_isr(void)
{
	DINT;

	if(DIP1 && DIP2) // Check Answer
	{
		ShowAnswerState();
	}
	else if(DIP1 && !DIP2) // Increase User Input
	{
		if(UserInputAdder == 100 && UserInput10 == 0 && UserInput1 == 0)
			UserInput100 = ++UserInput100 % 2;
		else if(UserInputAdder == 10 && GetUserInput() < 100)
			UserInput10 = ++UserInput10 % 10;
		else if (UserInputAdder = 1 && GetUserInput() < 100)
			UserInput1 = ++UserInput1 % 10;
	}
	else if(!DIP1 && DIP2) // Change where to increase on Input
	{
		UserInputAdder *= 10;
		if(UserInputAdder > 100) UserInputAdder = 1;

	}
	else if(!DIP1 && !DIP2)
	{
		if(UserInputAdder < 10) UserInputAdder = 100;
		else UserInputAdder /= 10;
	}

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void cpu_timer0_isr(void)
{
	DINT;
	if(!GameOffTrigger)
	{
		ShowTime();

		AddTick(-1);
		if(Tick <= 0)
		{
			GameState = TimeOut;
			ShowGameState();
		}

		ShowUserInput();
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

	ShowTime();
	ShowUserInput();

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

//=================================== Custom Function =========================================

inline void AddTick(int tick)
{
	Tick += tick;
}

inline void ResetTick()
{
	Tick = GAME_TIME_LIMIT;
}

int GetRandomNumberBetween(int from, int to)
{
	srand(Tick); // ÀÇ»ç ·£´ý ÇÔ¼ö·Î ¸¸µé¾î¾ßÇÔ

	return rand() % (to - from + 1) + from;
}

void ShowTime()
{
	lcd_write(LINE1 + 1, 0); // Move Cursor
	int millisecond = Tick % 10; // 0.1s
	int second = (int)(Tick / 10) % 60; // 1s
	int minute = (int)(Tick / 600) % 60; // 1m

	if(Tick < 0)
	{
		millisecond = 0; // 0.1s
		second = 0; // 1s
		minute = 0; // 1m
	}

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

void InitializeGame()
{
	// Clear
	lcd_write(0x01,0);		//Display Clear
	DELAY_US(1960);
	GameOffTrigger = 0;
	UserInputAdder = 1;

	// Set random target number
	TargetNumber = GetRandomNumberBetween(1, 100);
	ShowAnswer();

	// Start timer
	ResetTick();

	// Restart game
	UserInput100 = 0;
	UserInput10 = 0;
	UserInput1 = 0;

}

void ShowUserInput()
{
	lcd_write(LINE1 + LINE2 + 1, 0); // Move Cursor

	if(UserInput100 * 100 + UserInput10 * 10 + UserInput1 >= 0)
	{
		lcd_write((char)(UserInput100 + 48), 1);
		lcd_write((char)(UserInput10 + 48), 1);
		lcd_write((char)(UserInput1 + 48), 1);
	}
	else
	{
		int i = 0;
		for(i = 0; i < 3; i++)
			lcd_write((char)48, 1);
	}
}

void ShowString(char* str)
{
	lcd_write(LINE1 + LINE2 + 5, 0); // Move Cursor

	lcdprint_data(str);
}

void ShowGameState()
{
	if(GameState == GameWin)
		ShowString("You Win");
	else if(GameState == TimeOut)
		ShowString("Time Out");
}

void ShowAnswerState()
{
	if(TargetNumber == UserInput100 * 100 + UserInput10 * 10 + UserInput1)
	{
		ShowString("Success");
		DELAY_US(1000000);
		ShowString("You Win!!");
		DELAY_US(1000000);
		InitializeGame();

	}
	else
		ShowString("Fail");
}

void ShowAnswer()
{
	lcd_write(LINE1 + LINE2 + 13, 0); // Move Cursor

	// minute
	lcd_write((char)(TargetNumber / 100 + 48), 1);
	lcd_write((char)(TargetNumber % 100 / 10 + 48), 1);
	lcd_write((char)(TargetNumber % 10 + 48), 1);
}

inline int GetUserInput()
{
	return UserInput100 * 100 + UserInput10 * 10 + UserInput1;
}
