#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 �� HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 �� LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 �� HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 �� LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 �� ���簪�� ����
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 �� ���簪�� ����

#define DELAY_MAX 1000000
#define DELAY_MIN 0
#define DELAY_ALPHA 100000

void Gpio_select(void);

void main(){

	InitSysCtrl();        //basic core initialization

	DINT;                //Disable all interrupts

	Gpio_select();

	LED1_L;
	LED2_L;		//LED OFF(�ʱ�ȭ)

	double delay, alpha;
	for(delay = DELAY_MAX, alpha = -DELAY_ALPHA;; delay += alpha){
		LED1_T; LED2_T;
		DELAY_US(delay);

		if(delay > DELAY_MAX || delay < DELAY_MIN)
			alpha *= -1;
	}
}

void Gpio_select(void)
{
   EALLOW; // ��ȣ ����(�������͸� ����� �� ��ȣ�� Ǯ���ٰ� �ٽ� �ƿ����־����)
	GpioCtrlRegs.GPAMUX2.all = 0; //GPIO16...gpio31


	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;	//OUTPUT ����
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;	//OUTPUT ����
	
   EDIS; // ��ȣ ���

}


