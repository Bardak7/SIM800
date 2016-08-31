#include <stdio.h>
#include "phisic.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "SIM800.h"

struct sim800_current_state state_of_sim800_num1; // ������� ����� ���� ���������

//const struct request AT_req = {"AT", sim800_AT_responce_handler}; // ������� AT �������� �����
//const struct request ATplusCMGS_req = {"AT+CMGS=", sim800_ATplusCMGS_responce_handler}; // ������� AT+CMGS=�����������ջ - �������� ���

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
inline void Sim800_WriteCmd(struct sim800_current_state *current_state)
{
	current_state->send_uart_function(current_state->current_cmd); //� ����������� ����������� �������� \r
}

// ������� �������� ��� � UART �� ������� ����� Sim800
inline void Sim800_WriteSMS(struct sim800_current_state *current_state)
{
	current_state->send_uart_function(current_state->send_SMS_data); //� ����������� ����������� �������� \r
}

// ������� �������� �������� � SIM800
// ������� �������� �������� ��-������ ������ �������� � ��������� sim800_current_state ��� ������ ������ SIM800 ������ ������� ���������� ���������
// ����� �������� � ��� �� ��������� ��������� �� ���������� ������
// ����� �������� � ��� �� ��������� �������, ��� ������ ��������� (resp_rec)
// � ������ ����� �������� ������ � UART �� ������� ����� SIM800
// ���������: ��������� �� ��������� ���������� ������� ��������� ����������� ������ SIM800(�� ����� ���� ���������)
// ���������� ������ ����� (����������� ��������� ������ �����), ��� ������ ��������� ���� ������
int8_t sim800_request(struct sim800_current_state *current_state)
{
    if(current_state->communication_stage != proc_completed)
	    return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������

	current_state->communication_stage = resp_rec; // ���������� ���� ���������
	Sim800_WriteCmd(current_state);

	return exit_and_wait;                         // ������������ � ��������� ��� ������ ����� ������ � ���������� ������� ������, ������ ���������
}

//**********************************************************************
//**********************************************************************
//������� �������� �������� � ����������� ������� (������� ������/�����)

// ������� �������� ������� �� ������������� baudrate ������ SIM800 (������� "AT")
void sim800_AT_request(struct sim800_current_state * current_state)
{
    memcpy(current_state->current_cmd, "AT", 3);
    current_state->response_handler = sim800_AT_responce_handler;
	sim800_request(current_state); // AT\r ������� ������������� �������� �������� ������ SIM800 �� UART
}

// ���������� ������ ������� "AT"
void sim800_AT_responce_handler(struct sim800_current_state * current_state)
{
	if (strstr(current_state->responce,"AT")) // ������ ���?
	{
	    return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
	}
	else if (strstr(current_state->responce,"OK"))
	{
		current_state->result_of_last_execution = OK;
		current_state->response_handler = NULL; // ���������� ��������� �� ���������� � NULL (����� ���������)
		current_state->communication_stage = proc_completed;
		return;
	}
	else if (strstr(current_state->responce,"ERROR"))
	{
		current_state->result_of_last_execution = fail;
		current_state->response_handler = NULL;
		current_state->communication_stage = proc_completed;
		return;
	}
	else
	{
		current_state->unex_resp_handler(current_state);
		return;
	}
}

// ������� �������� ��� SIM800 (������� "AT+CMGS=�����������ջ")
void sim800_ATplusCMGS_request(struct sim800_current_state * current_state, uint8_t * phone_number, uint8_t * SMS_data)
{
    memcpy(current_state->current_cmd, "AT+CMGS=", 9);
    strncat(current_state->current_cmd, "\"", 16);
    strncat(current_state->current_cmd, phone_number, 14); // 14 - ������� � ������� �� ������ �������� (������� ���������� ����� �������� 11 �������)
    strncat(current_state->current_cmd, "\"", 14); // ������� ������� � ����� � ������ ����������� ������ ��� ���� ������� �������� AT-�������
    current_state->response_handler = sim800_ATplusCMGS_responce_handler;
	sim800_request(current_state); // AT\r ������� ������������� �������� �������� ������ SIM800 �� UART
}

// ���������� ������ ������� "AT+CMGS=�����������ջ"
void sim800_ATplusCMGS_responce_handler(struct sim800_current_state * current_state)
{
	if (strstr(current_state->responce,"AT+CMGS=")) // ������ ���?
	{
	    return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
	}
	else if (strstr(current_state->responce,">")) // ������ ����������� ������ ����� ��� ���������
	{
		Sim800_WriteSMS(current_state); // ���� ���-��������� ����� ������ current_state � ������� Sim800_WriteSMS �������� ��� �� ����
		return;
	}
	else if (strstr(current_state->responce,"OK"))
	{
		current_state->result_of_last_execution = OK;
		current_state->response_handler = NULL; // ���������� ��������� �� ���������� � NULL (����� ���������)
		current_state->communication_stage = proc_completed;
		return;
	}
	else if (strstr(current_state->responce,"ERROR"))
	{
		current_state->result_of_last_execution = fail;
		current_state->response_handler = NULL;
		current_state->communication_stage = proc_completed;
		return;
	}
	else
	{
		current_state->unex_resp_handler(current_state);
		return;
	}
}

//**********************************************************************
//**********************************************************************

// ������� ������������� ������ �� ������� SIM800
// ���������: 1) ��������� �� ���������� ��������� ����������� ��������� ����������� ������
//            2) ��������� �� ������� �������� ������ � ���������� UART �� ������� ����� ������ ������
uint8_t sim800_init(struct sim800_current_state * current_state, void (*send_uart_function)(char *))
{
	// �������������� ��������� ������� ��� ������ ����������� ������
	current_state->communication_stage = proc_completed;
	current_state->current_pos = 0;
	memset(current_state->rec_buf,'\0',REC_BUF_SIZE);
	memset(current_state->responce,'\0',REC_BUF_SIZE);
	memset(current_state->current_cmd,'\0',CURRENT_CMD_SIZE);
	//current_state->current_req = NULL;
	current_state->send_uart_function = send_uart_function; // �������� ����������� ������ � ����������� UART-�
	current_state->result_of_last_execution = OK;
	current_state->num_of_sms = 0;
	current_state->unex_resp_handler = unexpec_message_parse;
	current_state->response_handler = NULL;
	memset(current_state->send_phone_number,'\0',PHONE_NUM_SIZE);
	memset(current_state->rec_phone_number,'\0',PHONE_NUM_SIZE);
	memset(current_state->send_SMS_data,'\0',SEND_SMS_DATA_SIZE);
	memset(current_state->rec_SMS_data,'\0',REC_SMS_DATA_SIZE);
	//��� ���� ��� ������������������� � ������� ��� ������ ���-��
//	sim800_AT_request(current_module);
//	while (current_module->communication_stage);
//    if(current_module->result_of_last_execution)
//    	return 1; //�������
//    return 0;
}

//// ��������� ���
//void process_echo(uint8_t is_responce, uint8_t current_pos, struct sim800_current_state *current_state)
//{
//    //������ ���� ���������, � ��� ����� ����� �� ��� ������ (��������� �� ������������ ������� � ���������� ���)
//    if (strstr(current_state->rec_buf ,current_state->current_req->current_cmd))
//    {
//        // ���� ��, �� �������� ��������� ������� ������� ��������� ������ � ������, ��� ����� ������� ��� �� ����������� ��������
//        //(����� ������ ����� ������������ ������ ���), ��� �� �� ������ ����� \r\r\n �������� � ������� �������� ������
//       current_pos = 0;
//        // ���������� ��� ��������� ����, ��� ��� ���������� ���������� ������ �� ��������� ������������������ �������� \r\n - ��� �����
//        // ����� �� �������� ������, � �� ��������� ��������� �������� � ��������� SMS
//        is_responce = yes;
//    } // ���� �� ���, �� �������� ��� ��� �����-�� �������� �����
//    else // ��������� ����� ������ ������� \r ������ ����� ������ \0 (� ����� ���� \r\r\n ������ \0\r\n)
//    {
//        current_state->rec_buf[current_pos - 3] = '\0';
//
//
//        current_pos = 0;
//    }
//}
//
//// ��������� ������ �� �������
//void process_cmd(uint8_t is_responce, uint8_t current_pos, struct sim800_current_state *current_state)
//{
//    if( !strstr(&current_state->rec_buf[current_pos - 2], "\r\n") )
//        return; //����� �������
//
//    // ��� ����� ��������� ������ ������� \r ������ ����� ������ \0 (� ����� ���� \r\n ������ \0\n)
//    current_state->rec_buf[current_pos - 2] = '\0';
//    if (is_responce == yes) // ���� �� ����� ��� ������ ��� ������������ �������, �� ��� ����� �� ���
//    {
//        memcpy(current_state->responce, current_state->rec_buf, current_pos - 1);
//        //!!!!!!!!!
//        is_responce = no;
//    }
//    else // ���� �� ��� ��������� ���������, �������� � ������� SMS
//    {
//
//
//    }
//}

// ������� ���������� �� ����������� ���������� �� ������ �������� �� SIM800
// ������ ��������� ������ ������ � ���������� ��� �� ���������� ������ � ���������
// ���������: ��������� �� ��������� ���������� ������� ��������� ����������� ������ SIM800(�� ����� ���� ���������) � �������� �� UART ������
// �� ���� �� ����������, � � ���� ���������� ����� � �������� ����� � � ������ ������ ���� ������� �������� �������������� ���������� ������
void sim800_response_handler(struct sim800_current_state * current_state, uint8_t data)
{
    current_state->rec_buf[current_state->current_pos] = data;
    current_state->current_pos++;
    current_state->rec_buf[current_state->current_pos] = '\0'; // ��������� ��������� �� ��������� �������� �������� ������� ����-������������ ��� ���������� ������ ������� ��������� strstr, memcpy � ������
    if (current_state->current_pos > REC_BUF_SIZE - 1)
    {
        // � ������ ������������ ����������� ���������� �����
    	current_state->current_pos = 0;
        return;
    }
    if (current_state->current_pos <= 3)
        return;

    // ������ ���� ��������� �������� ����� �� ������� �������� \r\n
    if( strstr(&current_state->rec_buf[current_state->current_pos - 2], "\r\n") )
    {
    	memcpy(current_state->responce, current_state->rec_buf, current_state->current_pos); // �������� ���������� ��������� ������ � ����� ��������� ������ (�.�. ���������� ��������� ������ ���������� ���������� ��� ������)
    	if (current_state->response_handler != NULL)
    	{
    		current_state->communication_stage = resp_rec; // ����� �������
    		current_state->response_handler(current_state);
    	}
    	else // ���� �� ��������� �� ���������� ������ ����� NULL, ������ ������ �� ����������� � ��� ����������� ��������� �� SIM800 (�������� ������ ���)
    	{
    		current_state->unex_resp_handler(current_state);
		}
    	current_state->current_pos = 0;
    	return;
    }

}

//������� �������� ��������� ��������� �� SIM800 (�������� ������ SMS)
void unexpec_message_parse(struct sim800_current_state *current_state)
{
	if (strstr(current_state->responce,"+CMGS:")) // ��� ������������
	{
	    return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
	}
	else if (strstr(current_state->responce,"+CMTI:")) // ������ ��� ��������� (������� "+CMTI: "SM",12")
	{
		// !!!!!!!!! ��� ���� ������� ��������� ��������� ��� ��������� ��� ������ ���������������� ������� �������� ���������
		return;
	}
	else
	{
		return; // � ��� ��� ��������� ��������� ���������
	}
}

// ������� �������� SMS � ������ SIM800
//uint8_t sim800_sendSMS(uint8_t* text_buf, uint8_t length)
//{
//
//}
