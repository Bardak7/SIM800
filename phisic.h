#include <stdio.h>

#ifndef PHISIC_H_INCLUDED
#define PHISIC_H_INCLUDED

#define GPIO_SIM800_PWRKEY     GPIOA
#define SIM800_PWRKEY          GPIO_Pin_1
#define SIZE_BUF_UART1         32
#define SIZE_BUF_UART2         32

//extern uint8_t rec_buf_usart1[SIZE_BUF_UART1];  // ����� ��� ����������� ������ UART1
//extern int8_t rec_buf_last_usart1; // ������ ���������� ��������������� ������� � ������ UART1
//extern uint8_t rec_buf_usart1_overflow; //���� ������������ ��������� ������

//extern uint8_t rec_buf_usart2[SIZE_BUF_UART2];  // ����� ��� ����������� ������ UART2
//extern uint8_t rec_buf_last_usart2; // ������ ���������� ��������������� ������� � ������ UART2
//extern uint8_t rec_buf_usart2_overflow; //���� ������������ ��������� ������

void SetupClock(void);  // ������� ��������� ������������
void SetupUSART1(void); // ������� ��������� UART1
void SetupUSART2(void); // ������� ��������� UART2
void InitADC(void);     // ������� ��������� ���
void SetupGPIO(void);   // ������� ��������� GPIO

#endif
