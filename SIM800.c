#include <stdio.h>
#include "phisic.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"

// �������� ������ � UART �� ������� ����� SIM800
PutsToSim800(char * cmd)
{
	send_str_uart2rn(cmd); // ������� ������ � UART � ����������� ����������� �������� \n\r
}

// ������� �������� ������� � ������ Sim800
void Sim800_WriteCmd (const char *cmd)
{
    PutsToSim800(cmd); // �������� ������ � UART �� ������� ����� SIM800
}

// ������� �������� ������� � ������ Sim800 � ��������� ������ (������������� �������)
void Sim800_WriteCmd_confirm (const char *cmd)
{
    PutsToSim800(cmd); // �������� ������ � UART �� ������� ����� SIM800
    return Sim900_CompareStr("\r\nOK"); /// ������� ������ ������ � ����� UART � ��������� ������
}

// ������� ������������� ������ SIM800
uint8_t sim800_init(uint8_t init_data)
{

}

// ������� �������� SMS � ������ SIM800
uint8_t sim800_sendSMS(uint8_t* text_buf, uint8_t length)
{

}

// ������� ��������� ������ SIM800
void sim800_PWRKEY_on(void)
{
	// ������ �� ���� PWRKEY ���. 0
	GPIO_WriteBit (GPIO_SIM800_PWRKEY, SIM800_PWRKEY, Bit_RESET);
	// �������� � ��., ����������� ��� ���������
	DelayMs(2000);

	// ������ �� ���� PWRKEY ���. 1
	GPIO_WriteBit (GPIO_SIM800_PWRKEY, SIM800_PWRKEY, Bit_SET);
	// ������� ���������/���������� SIM800
	DelayMs(4000);

}
