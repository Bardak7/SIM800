#ifndef PHISIC_H_INCLUDED
#define PHISIC_H_INCLUDED

#define GPIO_SIM800_PWRKEY     GPIOA
#define SIM800_PWRKEY          GPIO_Pin_1
#define SIZE_BUF_UART1         32
#define SIZE_BUF_UART2         32

void SetupClock(void);  // ������� ��������� ������������
void SetupUSART1(void); // ������� ��������� UART1
void SetupUSART2(void); // ������� ��������� UART2
void InitADC(void);     // ������� ��������� ���
void SetupGPIO(void);   // ������� ��������� GPIO

#endif
