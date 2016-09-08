//Includes ------------------------------------------------------------------*/

#include "main.h"
#include "phisic.h"
#include "SIM800.h"

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include <stdio.h>

int i;

//**********************************************************************************************************
void Sys_Init(void) // ������� �������������� ������������� �������
{
    SetupClock();
    SetupGPIO();
    SetupUSART1();
    SetupUSART2();
    InitADC();
}

int main(void)
{
    Sys_Init(); // �������������� ������������� �������


    // ������������� ������� SIM800
    sim800_init(&state_of_sim800_num1, send_str_uart2); // ������ SIM800 ����� �� UART2


    volatile int i,j;

    for(i=0;i<0x1000000;i++);
    {
        for(j=0;j<0x500000;j++);
    }

    sim800_ATplusCMGF_request(&state_of_sim800_num1, text_mode); // ������������ � ��������� ����� ����� SMS

    for(i=0;i<0x1000000;i++);
    {
        for(j=0;j<0x500000;j++);
    }

    //    sim800_ATplusCMGD_request(&state_of_sim800_num1, 1, 4); // �������� ���� SMS
    //
    //	for(i=0;i<0x1000000;i++);
    //	{
    //	    for(j=0;j<0x500000;j++);
    //	}

    //sim800_ATplusCMGS_request(&state_of_sim800_num1, "+79198364844", "TEST!"); // �������� SMS
    //for(i=0;i<0x2000000;i++);
    //	{
    //	    for(j=0;j<0x500000;j++);
    //	}

    sim800_ATplusCMGR_request(&state_of_sim800_num1, 1, 0); // ������ SMS ��� ������� 1

    for(i=0;i<0x1000000;i++);
    {
        for(j=0;j<0x500000;j++);
    }

    sim800_ATplusCMGS_request(&state_of_sim800_num1, "+79198364844", state_of_sim800_num1.rec_SMS_data); // �������� ��������� SMS

//    if (strstr(state_of_sim800_num1.rec_SMS_data,"KAS"))
//    {
//    	GPIOA->ODR &= ~GPIO_Pin_0; // �������!!!
//    }

    while(1)
    {

        //sim800_AT_request(&state_of_sim800_num1);

        for(i=0;i<0x50000;i++);
        {
            for(j=0;j< 0x500000;j++);
        }
        /* Toggle LEDs which connected to PC6*/



        /* delay */
        //for(i=0;i<0x100000;i++);

        /* Toggle LEDs which connected to PC9*/

        //GPIOA->ODR ^= GPIO_Pin_0;

        /* delay */
        // for(i=0;i<0x100000;i++);
    }
}
