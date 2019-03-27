#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#define LED1	GpioDataRegs.GPADAT.bit.GPIO25
#define LED2	GpioDataRegs.GPADAT.bit.GPIO24
#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 핀 HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 핀 LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 핀 HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 핀 LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 핀 현재값에 반전
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 핀 현재값에 반전
#define DIP1	GpioDataRegs.GPBDAT.bit.GPIO48
#define DIP2	GpioDataRegs.GPBDAT.bit.GPIO49

#define SECOND_CHPATER

#ifdef FIRST_CHAPTER
#define DELAY_MAX 1000000
#define DELAY_MIN 0
#define DELAY_ALPHA 100000
#endif

#ifdef SECOND_CHPATER
#define DELAY 50000
#endif

void Gpio_select(void);

void main(){

	InitSysCtrl();        //basic core initialization

	DINT;                //Disable all interrupts

	Gpio_select();

	/** First Chapter
	 * LED blinks faster and slower
	LED1_L;
	LED2_L;		//LED OFF(초기화)

	double delay, alpha;
	for(delay = DELAY_MAX, alpha = -DELAY_ALPHA;; delay += alpha){
		LED1_T; LED2_T;
		DELAY_US(delay);

		if(delay > DELAY_MAX || delay < DELAY_MIN)
			alpha *= -1;
	}
	*/

	/** Second Chapter */
	for(;;)
	{
		LED1 = DIP1;
		LED2 = DIP2;

		DELAY_US(DELAY);
		if(DIP1 && !DIP2) LED1_T;
		else if(!DIP1 && DIP2) LED2_T;
		DELAY_US(DELAY);
	}
}

void Gpio_select(void)
{
   EALLOW; // 보호 해제(레지스터를 사용할 때 보호를 풀었다가 다시 아요해주어야함)

    GpioCtrlRegs.GPBPUD.bit.GPIO48 = 0;
   	GpioCtrlRegs.GPBPUD.bit.GPIO49 = 0;

	GpioCtrlRegs.GPAMUX2.all = 0; //GPIO16...gpio31

	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;	//OUTPUT 설정
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;	//OUTPUT 설정
	GpioCtrlRegs.GPBDIR.bit.GPIO48 = 0;
	GpioCtrlRegs.GPBDIR.bit.GPIO49 = 0;
	
   EDIS; // 보호 사용
}
