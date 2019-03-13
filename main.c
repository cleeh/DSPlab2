#include "DSP2833x_Device.h"     // DSP2833x Headerfile Include File
#include "DSP2833x_Examples.h"   // DSP2833x Examples Include File

#define LED1_H	GpioDataRegs.GPASET.bit.GPIO25 = 1		//GPIO25 ÇÉ HIGH
#define LED1_L	GpioDataRegs.GPACLEAR.bit.GPIO25 = 1	//GPIO25 ÇÉ LOW
#define LED2_H	GpioDataRegs.GPASET.bit.GPIO24 = 1		//GPIO24 ÇÉ HIGH
#define LED2_L	GpioDataRegs.GPACLEAR.bit.GPIO24 = 1	//GPIO24 ÇÉ LOW
#define LED1_T	GpioDataRegs.GPATOGGLE.bit.GPIO25 = 1	//GPIO25 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü
#define LED2_T	GpioDataRegs.GPATOGGLE.bit.GPIO24 = 1	//GPIO24 ÇÉ ÇöÀç°ª¿¡ ¹ÝÀü

void Gpio_select(void);

void main(){

	InitSysCtrl();        //basic core initialization

	DINT;                //Disable all interrupts

	Gpio_select();

	LED1_L;
	LED2_L;		//LED OFF(ÃÊ±âÈ­)

	for(;;)
	{
		LED1_T;
		LED2_T;				//LED ON/OFF
		DELAY_US(200000);	//1ÃÊ Áö¿¬
	}
}

void Gpio_select(void)
{
   EALLOW;
	GpioCtrlRegs.GPAMUX2.all = 0; //GPIO16...gpio31


	GpioCtrlRegs.GPADIR.bit.GPIO25 = 1;	//OUTPUT ¼³Á¤
	GpioCtrlRegs.GPADIR.bit.GPIO24 = 1;	//OUTPUT ¼³Á¤
	
   EDIS;

}


