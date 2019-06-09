#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "stdlib.h"
#include "math.h"

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

#define LCD_ROW 2
#define LCD_COLUMN 16
#define LINE1 0x80
#define LINE2 0x40

/* Interrupt Vector */
interrupt void XINT1_isr(void);
interrupt void XINT2_isr(void);
interrupt void cpu_timer0_isr(void);
interrupt void cpu_timer1_isr(void);
interrupt void cpu_timer2_isr(void);

/* Initial Setting */
void Gpio_select(void);
void lcdprint_data(char *str);
void lcd_write(char data,unsigned char Rs);
void lcd_Gpio_data_out(unsigned char da);
void lcd_init(void);

/* Custom Function */
int MoveCursor(int row, int column);
int WriteOnLCD(char* str, int row, int column);
void WriteNumberOnLCD(int number, int row, int column);
void ClearOnLCD();
int GetRandomNumberBetween(int from, int to);

void InitializeGame();
void SetGameTime();
void SwitchGameMode();
void StartGame();
void ShowPreviousResults();
void GetRPSInput();
void CheckWinner();
void GetRPS7Input();
void CheckRemainingGameTime();
int CheckSuperior(ERPS player, ERPS other);

/* Variables */
typedef enum{
	RPS,
	RPS_7
}EGameMode;

typedef enum{
	Rock,
	Fire,
	Scissors,
	Sponge,
	Paper,
	Air,
	Water,
	None = -100
}ERPS;

unsigned const int InputTickLimit;

unsigned int Tick = 0;

EGameMode GameMode = RPS;
unsigned int WinNumber = 0;
unsigned int LoseNumber = 0;
unsigned int DrawNumber = 0;

ERPS PlayerRPSInput = None;
ERPS OtherRPSInput = None;

interrupt void XINT1_isr(void)
{
	DINT;

	// Game Setting
	if(!DIP1 && !DIP2) // Initialize Game
	{
		InitializeGame();
	}
	else if(DIP1 && DIP2) // Select Game Mode
	{
		SwitchGameMode();
	}

	// RPS
	else if(DIP1 && !DIP2) // R/P/S Input
	{
		GetRPSInput();
	}

	// RPS-7
	else // R/W/A/P/Sp/Sc/F Input
	{
		GetRPS7Input();
	}

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void XINT2_isr(void)
{
	DINT;

	// Game Setting
	if(!DIP1 && !DIP2) // Set Game Time
	{
		SetGameTime();
	}
	else if(DIP1 && DIP2) // Start Game / Check Previous Results
	{
		StartGame();
		ShowPreviousResults();
	}

	// RPS
	else if(DIP1 && !DIP2) // Check Winner
	{
		CheckWinner();
	}
	// RPS-7
	else // Check Remaining Game Time
	{
		CheckRemainingGameTime();
	}

	EINT;
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

interrupt void cpu_timer0_isr(void)
{
	DINT;

	Tick++;

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

	WriteNumberOnLCD(-1234, 0, 0);

	while(1);
}
//=================================== Custom Function =========================================
int MoveCursor(int row, int column)
{
	if(column < 0 || column >= LCD_COLUMN || row < 0 || row >= LCD_ROW) return 0;

	lcd_write(LINE1 + LINE2 * row + column, 0);
	return 1;
}

int WriteOnLCD(char* str, int row, int column)
{
	if(MoveCursor(row, column))
	{
		lcdprint_data(str);
		return 1;
	}

	return 0;
}

void WriteNumberOnLCD(int number, int row, int column)
{
	unsigned int str_index = 0;
	char str[16] = { NULL };
	if(number < 0)
	{
		number *= -1;
		str[str_index++] = '-';
	}

	int divider;
	for(divider = 10000; divider > 1; divider /= 10)
	{
		if(number / divider)
			str[str_index++] = number / divider % 10 + 48;
	}

	if(number == 0) str[0] = '0';
	MoveCursor(row, column);
	lcdprint_data(str);
}

void ClearOnLCD()
{
	WriteOnLCD("                ", 0, 0);
	WriteOnLCD("                ", 1, 0);
}

int GetRandomNumberBetween(int from, int to)
{
	srand(Tick); // ÀÇ»ç ·£´ý ÇÔ¼ö·Î ¸¸µé¾î¾ßÇÔ

	return rand() % (to - from + 1) + from;
}

//========== Game Function
void InitializeGame()
{
	ClearOnLCD();

	Tick = 0;

	GameMode = RPS;

	WinNumber = 0;
	LoseNumber = 0;
	DrawNumber = 0;

	PlayerRPSInput = None;
}

void SetGameTime()
{
	WriteNumberOnLCD(60, 0, 11);
	WriteNumberOnLCD(60, 0, 14);
}

void SwitchGameMode()
{
	ClearOnLCD();

	// Display Frame
	WriteOnLCD("SELECT MODE", 0, 2);
	WriteOnLCD("-RPS -PRS-7", 1, 2);

	switch(GameMode)
	{
	case RPS:
		GameMode = RPS_7;
		MoveCursor(1, 2);
		break;
	case RPS_7:
		GameMode = RPS;
		MoveCursor(1, 7);
		break;
	}
}

void StartGame()
{
	ClearOnLCD();

	WriteOnLCD("1.RPS MODE", 0, 3);
	WriteOnLCD("YOU:s       PC:*", 1, 0);
}

void ShowPreviousResults()
{
	ClearOnLCD();

	WriteOnLCD("Total game#: ", 0, 0);
	WriteNumberOnLCD(WinNumber + DrawNumber + LoseNumber, 0, 12);
	WriteOnLCD("W:", 1, 0);
	WriteNumberOnLCD(WinNumber, 1, 2);
	WriteOnLCD("D:", 1, 5);
	WriteNumberOnLCD(DrawNumber, 1, 7);
	WriteOnLCD("L:", 1, 10);
	WriteNumberOnLCD(LoseNumber, 1, 12);
}

void GetRPSInput()
{
	swtich(PlayerRPSInput)
	{
	case Rock:
		PlayerRPSInput = Scissors;
		WriteOnLCD('S', 1, 4);
		break;
	case Scissors:
		PlayerRPSInput = Paper;
		WriteOnLCD('P', 1, 4);
		break;
	default:
		PlayerRPSInput = Rock;
		WriteOnLCD('R', 1, 4);
		break;
	}
}

void CheckWinner()
{
	switch (OtherRPSInput)
	{
	case Rock:
		WriteOnLCD("R", 0, 14);
		break;
	case Fire:
		WriteOnLCD("F", 0, 14);
		break;
	case Scissors:
		WriteOnLCD("Sc", 0, 14);
		break;
	case Sponge:
		WriteOnLCD("Sp", 0, 14);
		break;
	case Paper:
		WriteOnLCD("P", 0, 14);
		break;
	case Air:
		WriteOnLCD("A", 0, 14);
		break;
	case Water:
		WriteOnLCD("W", 0, 14);
		break;
	default:
		break;
	}
}

void GetRPS7Input()
{
	switch (PlayerRPSInput)
	{
	case Rock:
		PlayerRPSInput = Fire;
		WriteOnLCD("F", 1, 4);
		break;
	case Fire:
		PlayerRPSInput = Scissors;
		WriteOnLCD("Sc", 1, 4);
		break;
	case Scissors:
		PlayerRPSInput = Sponge;
		WriteOnLCD("Sp", 1, 4);
		break;
	case Sponge:
		PlayerRPSInput = Paper;
		WriteOnLCD("P", 1, 4);
		break;
	case Paper:
		PlayerRPSInput = Air;
		WriteOnLCD("A", 1, 4);
		break;
	case Air:
		PlayerRPSInput = Water;
		WriteOnLCD("W", 1, 4);
		break;
	default:
		PlayerRPSInput = Rock;
		WriteOnLCD("R", 1, 4);
		break;
	}
}

void CheckRemainingGameTime()
{
	ClearOnLCD();
	WriteOnLCD("Total Time   :", 0, 0);
	WriteNumberOnLCD(60, 0, 11);
	WriteNumberOnLCD(60, 1, 14);
	WriteOnLCD("Re. Time     :", 0, 0);
	WriteNumberOnLCD(60, 0, 11);
	WriteNumberOnLCD(60, 0, 14);
}

int CheckSuperior(ERPS player, ERPS other)
{
	int difference = other - player;

	switch(difference)
	{
	// Case: Player is superior
	case 1:
	case 2:
	case 3:
	case -4:
	case -5:
	case -6:
		return 1;
		break;

	// Case: Other is superior
	case -1:
	case -2:
	case -3:
	case 4:
	case 5:
	case 6:
		return -1;
		break;

	// Case: Apposition each other
	case 0:
		return 0;
		break;

	default:
		ClearOnLCD();
		WriteOnLCD("Error: Checking superior", 0, 0);
		return -999;
		break;
	}
}
//================================== Initiali Setting =========================================

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
	lcd_write(0x0F,0);		//display on
	lcd_write(0x01,0);		//Display Clear
    DELAY_US(1960);
	lcd_write(0x06,0);		//Entry mode
}
