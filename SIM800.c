#include <stdio.h>
#include "phisic.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "SIM800.h"

sim800_current_state sim800_1_current_state; // ������� ����� ���� ���������

//sim800_1_current_state.send_uart_function = send_str_uart2rn; // ������ ���������� ������ SIM800 ����� �� ����� UART2

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

// ������� �������� ������ � UART �� ������� ����� Sim800
inline void Sim800_WriteCmd(sim800_current_state * current_module)
{
	current_module->send_uart_function(current_module->current_cmd); //� ����������� ����������� �������� \r\n
}

//������� �������� �������� � ����������� ������� (������� ������/�����)

// ������� �������� ������� �� ������������� baudrate ������ SIM800 (������� "AT")
void sim800_AT_request(sim800_current_state * current_module)
{
    memcpy(current_module->current_cmd, "AT", 2);
    current_module->response_handler = sim800_AT_responce_handler;
	sim800_request(current_module); // AT\r ������� ������������� �������� �������� ������ SIM800 �� UART
}

// ���������� ������ ������� "AT"
void sim800_AT_responce_handler(uint8_t * responce, uint8_t result, uint8_t is_busy)
{
	if (strstr(responce,"OK"))
	{
	    result = OK;
	}
	else
	{
		result = fail;
	}
	is_busy = free; // � ����� ��������� ������ �� ������ ���������� ���� "��������", ��� ���������� ����� ��������
}

// ������� ������������� ������ �� ������� SIM800
uint8_t sim800_init(sim800_current_state * current_module, int32_t * init_data)
{
	sim800_AT_request(current_module);
	while (current_module->communication_stage);
    if(current_module->result_of_last_execution)
    	return 1; //�������
    return 0;
}

// ������� �������� �������� � SIM800
// ���������: ��������� �� ��������� ���������� ������� ��������� ����������� ������ SIM800(�� ����� ���� ���������)
// ���������� ������ ����� �� �����
int8_t sim800_request(sim800_current_state * current_state)
{
    if(current_state->communication_stage)
	    return busy;

	current_state->communication_stage = busy;
	Sim800_WriteCmd(current_state->current_cmd);

	return exit_and_wait;                         // ������������ � ��������� ��� ������ ����� ������ � ���������� ������� ������
}

// ��������� ���
void process_echo(uint8_t is_responce, uint8_t current_pos, sim800_current_state *current_state)
{
    //������ ���� ���������, � ��� ����� ����� �� ��� ������ (��������� �� ������������ ������� � ���������� ���)
    if (strstr(current_state->rec_buf ,current_state->current_cmd))
    {
        // ���� ��, �� �������� ��������� ������� ������� ��������� ������ � ������, ��� ����� ������� ��� �� ����������� ��������
        //(����� ������ ����� ������������ ������ ���), ��� �� �� ������ ����� \r\r\n �������� � ������� �������� ������
       current_pos = 0;
        // ���������� ��� ��������� ����, ��� ��� ���������� ���������� ������ �� ��������� ������������������ �������� \r\n - ��� �����
        // ����� �� �������� ������, � �� ��������� ��������� �������� � ��������� SMS
        is_responce = yes;
    } // ���� �� ���, �� �������� ��� ��� �����-�� �������� �����
    else // ��������� ����� ������ ������� \r ������ ����� ������ \0 (� ����� ���� \r\r\n ������ \0\r\n)
    {
        current_state->rec_buf[current_pos - 3] = '\0';
        memcpy(current_state->unexp_responce, current_state->rec_buf, current_pos - 2);
        unexpec_message_parse(current_state->unexp_responce); // �������� ������� ������� ��������� ���������
        current_pos = 0;
    }
}

// ��������� ������ �� �������
void process_cmd(uint8_t is_responce, uint8_t current_pos, sim800_current_state *current_state)
{
    if( !strstr(&current_state->rec_buf[current_pos - 2], "\r\n") )
        return; //����� �������

    // ��� ����� ��������� ������ ������� \r ������ ����� ������ \0 (� ����� ���� \r\n ������ \0\n)
    current_state->rec_buf[current_pos - 2] = '\0';
    if (is_responce == yes) // ���� �� ����� ��� ������ ��� ������������ �������, �� ��� ����� �� ���
    {
        memcpy(current_state->responce, current_state->rec_buf, current_pos - 1);
        current_state->response_handler(current_state->responce, current_state->result_of_last_execution, current_state->communication_stage); // �������� ������� ��������� ������, ������� ��� ������� � ��������� �������� ���������
        is_responce = no;
    }
    else // ���� �� ��� ��������� ���������, �������� � ������� SMS
    {
        memcpy (current_state->unexp_responce, current_state->rec_buf, current_pos - 1);
        unexpec_message_parse(current_state->unexp_responce); // �������� ������� ������� ��������� ���������
    }
}

// ������� ���������� �� ����������� ���������� �� ������ �������� �� SIM800
// ������ ��������� ������ ������ � ���������� ��� �� ���������� ������ � ���������
// ���������: ��������� �� ��������� ���������� ������� ��������� ����������� ������ SIM800(�� ����� ���� ���������) � �������� �� UART ������
// �� ���� �� ����������, � � ���� ���������� ����� � �������� ����� � � ������ ������ ���� ������� �������� �������������� ���������� ������
void sim800_response_handler(sim800_current_state * current_state, uint8_t data)
{
    //static uint8_t rec_buf[REC_BUF_SIZE]; // ��������� �������� �����

    current_state->rec_buf[current_state->current_pos] = data;
    current_state->current_pos++;
    current_state->rec_buf[current_state->current_pos] = '\0';
    if (current_state->current_pos > REC_BUF_SIZE - 1)
    {
        // � ������ ������������ ����������� ���������� �����
    	current_state->current_pos = 0;
        return;
    }
    if (current_state->current_pos <= 3)
        return;

    // ������ ���� ��������� �������� ����� �� ������� ���� �������� \r\r\n (������� ����� ��� ������������ �������
    // ���� ������� ��������������� �����)
    // ���� �������� \r\n (�������� ����� ������ ����������� �� ������ SIM800)
    if( strstr(&current_state->rec_buf[current_state->current_pos - 3], "\r\r\n") )
    {

    	process_echo(current_state->is_responce, current_state->current_pos, current_state);
    	return;
    }
    process_cmd(current_state->is_responce, current_state->current_pos, current_state);
}

//������� �������� ��������� ��������� �� SIM800 (�������� ������ SMS)
void unexpec_message_parse(uint8_t * unexp_responce)
{

}

// ������� �������� SMS � ������ SIM800
//uint8_t sim800_sendSMS(uint8_t* text_buf, uint8_t length)
//{
//
//}
