﻿//Includes ------------------------------------------------------------------*/

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
void Sys_Init(void) // функция первоначальной инициализации системы
{
    SetupClock();
    SetupGPIO();
    SetupUSART1();
    SetupUSART2();
    InitADC();
}

int main(void)
{
	volatile int i,j;
	Sys_Init(); // первоначальная инициализация системы
    // инициализация первого SIM800
    sim800_init(&state_of_sim800_num1, send_str_uart2, 2, 6239); // Первый SIM800 сидит на UART2

//    // инициализация первого SIM800
//    sim800_init(&state_of_sim800_num1, send_str_uart2); // Первый SIM800 сидит на UART2
//
//
//
//    for(i=0;i<0x1000000;i++);
//    {
//        for(j=0;j<0x500000;j++);
//    }
//
//    sim800_ATplusCPIN_request(&state_of_sim800_num1, 6239);
//
//    for(i=0;i<0x1000000;i++);
//    {
//        for(j=0;j<0x500000;j++);
//    }
//
//    sim800_ATplusCMGF_request(&state_of_sim800_num1, text_mode); // переключение в текстовый режим ввода SMS
//
//    for(i=0;i<0x1000000;i++);
//    {
//        for(j=0;j<0x500000;j++);
//    }
//
//        sim800_ATplusCMGD_request(&state_of_sim800_num1, 1, 4); // удаление всех SMS
//
//    	for(i=0;i<0x1000000;i++);
//    	{
//    	    for(j=0;j<0x500000;j++);
//    	}
//
//    //sim800_ATplusCMGS_request(&state_of_sim800_num1, "+79198364844", "TEST!"); // отправка SMS
//    //for(i=0;i<0x2000000;i++);
//    //	{
//    //	    for(j=0;j<0x500000;j++);
//    //	}
//
//    sim800_ATplusCMGR_request(&state_of_sim800_num1, 1, 0); // чтение SMS под номером 1
//
//    for(i=0;i<0x1000000;i++);
//    {
//        for(j=0;j<0x500000;j++);
//    }
//
//    sim800_ATplusCMGS_request(&state_of_sim800_num1, "+79649955199", state_of_sim800_num1.rec_SMS_data); // отправка ответного SMS
//    //sim800_ATplusCMGS_request(&state_of_sim800_num1, "+79198364844", state_of_sim800_num1.rec_SMS_data); // отправка ответного SMS
//
//    //    if (strstr(state_of_sim800_num1.rec_SMS_data,"KAS"))
//    //    {
//    //    	GPIOA->ODR &= ~GPIO_Pin_0; // ОТЛАДКА!!!
//    //    }
//
//    for(i=0;i<0x1000000;i++);
//    {
//        for(j=0;j<0x500000;j++);
//    }
//
//    sim800_ATplusCUSD_request(&state_of_sim800_num1, MTS_balance_request);

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
