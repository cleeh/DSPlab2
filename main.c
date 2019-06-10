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

#define FNDA_H	GpioDataRegs.GPBSET.bit.GPIO50 = 1
#define FNDB_H	GpioDataRegs.GPBSET.bit.GPIO51 = 1
#define FNDC_H	GpioDataRegs.GPBSET.bit.GPIO52 = 1
#define FNDD_H	GpioDataRegs.GPBSET.bit.GPIO53 = 1
#define FNDA_L	GpioDataRegs.GPBCLEAR.bit.GPIO50 = 1
#define FNDB_L	GpioDataRegs.GPBCLEAR.bit.GPIO51 = 1
#define FNDC_L	GpioDataRegs.GPBCLEAR.bit.GPIO52 = 1
#define FNDD_L	GpioDataRegs.GPBCLEAR.bit.GPIO53 = 1

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
void Gpio_Fnd_out(unsigned char da);

/* Custom Function */
int MoveCursor(int row, int column);
int WriteOnLCD(char* str, int row, int column);
void WriteNumberOnLCD(int number, int row, int column);
void ClearOnLCD();
int GetRandomNumberBetween(int from, int to);

typedef enum{
	Setting,
	GamePlaying,
	CheckingTime,
	CheckingPreviousResults
}EGameState;

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
}ERPS;

/* Game Function */
void InitializeGame();
void SetGameTime();
void SwitchGameMode();
void StartGame();
void ShowPreviousResults();
void GetRPSInput();
void CheckWinner();
void GetRPS7Input();
void CheckRemainingGameTime();
void GameOver();
int CheckSuperior(ERPS player, ERPS other);
void NotifyInvalidOperation();
void WriteRPSOnLCD(ERPS input, int row, int column);
ERPS SetRandomOtherRPS();

/* Variables */
unsigned const int InputTickLimit = 5;

unsigned int FNDTickPointer = 5;
unsigned int GameTotalTick = 0;
unsigned int Tick = 0;
unsigned int GameTimeLimit = 20;
unsigned int RemainedGameTime = 20;

EGameState GameState = Setting;
EGameMode GameMode = RPS;
unsigned int TotalPlayNumber = 0;
unsigned int WinNumber = 0;
unsigned int LoseNumber = 0;
unsigned int DrawNumber = 0;

ERPS PlayerRPSInput = Rock;
ERPS OtherRPSInput = Rock;

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
		if(GameState == Setting)
			StartGame();
		else if(GameState == GamePlaying)
		{
			GameState = CheckingPreviousResults;
			ShowPreviousResults();
			DELAY_US(800000);
			StartGame();
		}
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
	if(GameState == GamePlaying && Tick % 10 == 0)
	{
		GameTotalTick++;
		RemainedGameTime--;
		if(RemainedGameTime == 0 || RemainedGameTime > 120)
			GameOver();
		if(FNDTickPointer > 0) FNDTickPointer--;
	}
	if(GameState == GamePlaying)
	{
		Gpio_Fnd_out(FNDTickPointer);
		if(FNDTickPointer == 0)
			TimeOver();
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

	InitializeGame();

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
	for(divider = 10000; divider >= 1; divider /= 10)
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

	FNDTickPointer = 5;
	GameTotalTick = 0;
	Tick = 0;
	GameTimeLimit = 20;
	RemainedGameTime = 20;

	GameState = Setting;
	GameMode = RPS;
	TotalPlayNumber = 0;
	WinNumber = 0;
	LoseNumber = 0;
	DrawNumber = 0;

	PlayerRPSInput = Rock;
	OtherRPSInput = Rock;

	WriteOnLCD("Initialized", 0, 0);
}

void SetGameTime()
{
	if(GameState == Setting)
	{
		ClearOnLCD();
		WriteOnLCD("Set Game Time", 0, 1);

		if(GameTimeLimit < 120) GameTimeLimit += 10;
		else GameTimeLimit = 20;
		WriteOnLCD(":", 1, 6);
		WriteNumberOnLCD(GameTimeLimit / 60, 1, 5);
		WriteNumberOnLCD(GameTimeLimit % 60, 1, 7);
		RemainedGameTime = GameTimeLimit;
	}
}

void SwitchGameMode()
{
	if(GameState == Setting)
	{
		ClearOnLCD();

		// Display Frame
		WriteOnLCD("SELECT MODE", 0, 2);
		WriteOnLCD("-RPS -PRS-7", 1, 2);

		switch(GameMode)
		{
		case RPS:
			GameMode = RPS_7;
			MoveCursor(1, 7);
			break;
		case RPS_7:
			GameMode = RPS;
			MoveCursor(1, 2);
			break;
		}
	}
}

void StartGame()
{
	ClearOnLCD();
	GameState = GamePlaying;

	if(GameMode == RPS) WriteOnLCD("1.RPS MODE", 0, 3);
	else if (GameMode == RPS_7) WriteOnLCD("2.RPS-7 MODE", 0, 2);
	WriteOnLCD("ME:        PC:*", 1, 0);
	WriteRPSOnLCD(PlayerRPSInput, 1, 3);
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
	if(GameState == GamePlaying && GameMode == RPS)
		switch(PlayerRPSInput)
		{
		case Rock:
			PlayerRPSInput = Scissors;
			WriteOnLCD("S", 1, 3);
			break;
		case Scissors:
			PlayerRPSInput = Paper;
			WriteOnLCD("P", 1, 3);
			break;
		default:
			PlayerRPSInput = Rock;
			WriteOnLCD("R", 1, 3);
			break;
		}
}

void CheckWinner()
{
	if(GameState != GamePlaying)
	{
		NotifyInvalidOperation();
		return;
	}

	SetRandomOtherRPS();

	WriteOnLCD("    ", 1, 6);
	switch (CheckSuperior(PlayerRPSInput, OtherRPSInput))
	{
	case 1: // Player is winner
		WriteOnLCD("Win", 1, 6);
		WinNumber++;
		TotalPlayNumber++;
		break;
	case -1: // Other is winner
		WriteOnLCD("Lose", 1, 6);
		LoseNumber++;
		TotalPlayNumber++;
		break;
	case 0: // Draw
		WriteOnLCD("Draw", 1, 6);
		DrawNumber++;
		TotalPlayNumber++;
		break;
	}
	WriteOnLCD("  ", 1, 14);
	WriteRPSOnLCD(OtherRPSInput, 1, 14);

	FNDTickPointer = 5;
}

void GetRPS7Input()
{
	if(GameState == GamePlaying && GameMode == RPS_7)
	{
		WriteOnLCD("  ", 1, 3);
		switch (PlayerRPSInput)
		{
		case Rock:
			PlayerRPSInput = Fire;
			WriteOnLCD("F", 1, 3);
			break;
		case Fire:
			PlayerRPSInput = Scissors;
			WriteOnLCD("Sc", 1, 3);
			break;
		case Scissors:
			PlayerRPSInput = Sponge;
			WriteOnLCD("Sp", 1, 3);
			break;
		case Sponge:
			PlayerRPSInput = Paper;
			WriteOnLCD("P", 1, 3);
			break;
		case Paper:
			PlayerRPSInput = Air;
			WriteOnLCD("A", 1, 3);
			break;
		case Air:
			PlayerRPSInput = Water;
			WriteOnLCD("W", 1, 3);
			break;
		default:
			PlayerRPSInput = Rock;
			WriteOnLCD("R", 1, 3);
			break;
		}
	}
}

void CheckRemainingGameTime()
{
	if(GameState == GamePlaying)
	{
		ClearOnLCD();
		WriteOnLCD("Total Time   :", 0, 0);
		if(GameTotalTick >= 600)
			WriteNumberOnLCD(GameTotalTick / 3600, 0, 11); // minute
		else
			WriteNumberOnLCD(GameTotalTick / 3600, 0, 12);
		WriteNumberOnLCD(GameTotalTick % 60, 0, 14); // second

		WriteOnLCD("Re. Time     :", 1, 0);
		WriteNumberOnLCD(RemainedGameTime / 60, 1, 12); // minute
		WriteNumberOnLCD(RemainedGameTime % 60, 1, 14); // second

		GameState = CheckingTime;
	}
	else if(GameState == CheckingTime)
		StartGame();
}

void TimeOver()
{
	WriteOnLCD("Lose", 1, 6);
	LoseNumber++;
	TotalPlayNumber++;

	FNDTickPointer = 5;
}

void GameOver()
{
	GameState = Setting;
	RemainedGameTime = GameTimeLimit;

	ClearOnLCD();
	WriteOnLCD("Game is Over", 0, 1);
	DELAY_US(800000);
	ShowPreviousResults();
}

void NotifyInvalidOperation()
{
	ClearOnLCD();
	
	WriteOnLCD("Invalid Operatio", 0, 0);
	WriteOnLCD("n.Return SEL.-M", 1, 0);
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

	// Case: Other is superior
	case -1:
	case -2:
	case -3:
	case 4:
	case 5:
	case 6:
		return -1;

	// Case: Apposition each other
	case 0:
		return 0;

	default:
		ClearOnLCD();
		WriteOnLCD("Error: Checking superior", 0, 0);
		DELAY_US(800000);
		return -999;
	}
}

void WriteRPSOnLCD(ERPS input, int row, int column)
{
	switch(input)
	{
	case Rock:
		WriteOnLCD("R", row, column);
		break;
	case Fire:
		WriteOnLCD("F", row, column);
		break;
	case Scissors:
		WriteOnLCD("Sc", row, column);
		break;
	case Sponge:
		WriteOnLCD("Sp", row, column);
		break;
	case Paper:
		WriteOnLCD("P", row, column);
		break;
	case Air:
		WriteOnLCD("A", row, column);
		break;
	case Water:
		WriteOnLCD("W", row, column);
		break;
	}
}

ERPS SetRandomOtherRPS()
{
	int output;

	srand(Tick);
	switch(GameMode)
	{
	case RPS:
		output = rand() % 6 / 2 * 2;
		break;
	case RPS_7:
		output = rand() % 7;
		break;
	}

	return (OtherRPSInput = (ERPS)output);
}
//================================== Initiali Setting =========================================

void Gpio_select(void)
{
	EALLOW;
	GpioCtrlRegs.GPAMUX2.all = 0x0000;
	GpioCtrlRegs.GPBMUX2.all = 0x0000;

	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;

	// FND Register
	GpioCtrlRegs.GPBDIR.bit.GPIO50 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO51 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO52 = 1;
	GpioCtrlRegs.GPBDIR.bit.GPIO53 = 1;

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
