// ������� ��� �������� � ������ ������ �� ���������� GSM
#include <stdio.h>
#include "stm32f10x_gpio.h"

#include "flash.h"
#include "REG74HC165.h"
#include "SIM800.h"
#include "GSMcommunication.h"

// ��������� ��������� ������� ��������� ���������������� GSM ����������
struct GSM_communication_state{
    uint8_t Status_of_mailing;                 // ������ ��������: ����� (busy)- �������� ��������� ����,
                                               //                  �������� (free) - �������� ��������� ���������,
    uint8_t status_mes_send;                   // ���� ������� �������� SMS ����� ��������� SMS_send_stop = 0 ��� SMS_send_start = 1
    uint8_t current_abonent;                   // ������� ����� �������
    uint8_t max_num_of_abonent;                // ������������ ����� ��������� ��� ��������
    uint8_t SMS_text[MAX_SIZE_STRING_8];       // ������� ���� SMS ��� ��������
    uint8_t phone_num[MAX_SIZE_STR_PHONE_8];   // ����� �������� �������� ��������
};

struct GSM_communication_state GSM_com_state; // ��������� �������� ������� ��������� ����������������� GSM ��������

// ������� ������������� ����������������� ����������
void GSM_Com_Init(struct sim800_current_state * GSMmod)
{
	GSM_com_state.SMS_text[0] = '\0';
	uint8_t status_mes_send = SMS_send_stop;
	GSM_com_state.phone_num[0] = '\0';
	GSM_com_state.Status_of_mailing = free;
	GSM_com_state.current_abonent = 0;
	GSM_com_state.max_num_of_abonent = NUM_OF_ABONENTS;
}

// ������� �������� SMS
// ������� �������� ��� ��������� ��������� �������������� �������������� �������
void sendSMS(void)
{
    do
    {
    	FLASH_Read_Phone_Num(GSM_com_state.current_abonent, GSM_com_state.phone_num, MAX_SIZE_STR_PHONE_8);
    	GSM_com_state.current_abonent++;
    	if (GSM_com_state.current_abonent == GSM_com_state.max_num_of_abonent) // ���� ����������� �� ���� ������� ���� ������ � ���������� ������� ������ ���
    	{
       		GSM_com_state.current_abonent = 0;
       		GSM_com_state.Status_of_mailing = free;
       		return;
    	}
    }
    while (GSM_com_state.phone_num[0] == 0xFF);
    GSM_com_state.current_abonent--;

    if ((state_of_sim800_num1.communication_stage == proc_completed)     // ���� GSM-������ �� ����� ���������� ����������� ������� �
   			&& (GSM_com_state.status_mes_send == SMS_send_stop))         // �� ��� �� ������ �������� SMS
   	{
     	GSM_com_state.status_mes_send = SMS_send_start; // ��������� ������ �� �������� SMS
   		sim800_ATplusCMGS_request(&state_of_sim800_num1, GSM_com_state.phone_num, GSM_com_state.SMS_text);

   		return;
   	}

    if (state_of_sim800_num1.communication_stage == proc_completed) // ���� GSM-������ �� ����� ���������� ����������� �������
    {
    	GPIOA->ODR ^= GPIO_Pin_0;
   		GSM_com_state.status_mes_send = SMS_send_stop;
   		if (state_of_sim800_num1.result_of_last_execution == OK)
   		{
   			GSM_com_state.current_abonent++; // ��������� �������
   		}
    	return;
    }

   	return;
}

// ������� ��������� �������� SMS
uint8_t recSMS(void)
{
    static uint8_t recSMSstatus; // ���� �������� ������� ������������ SMS-���������
    if ((recSMSstatus == free) && (state_of_sim800_num1.communication_stage == proc_completed)) // ���� SMS ��� �� �������� � ������ �������� ��������� ���� ���������� ��������
    {
    	recSMSstatus = busy;
    	sim800_ATplusCMGR_request(&state_of_sim800_num1, 1, 0); // ������ SMS ��� ������� 1 !!!!!!!!!!!!!!!!!!!!!
    }
    else if (state_of_sim800_num1.communication_stage == proc_completed)
    {

    	// �������� ��������� SMS
    	//!!!!!!!!!!GSM_com_state.sim800module->rec_SMS_data;
    	if (stristr(state_of_sim800_num1.rec_SMS_data,"BLA-BLA-BLA")) // ������ ��� ��������� � ������� BLA-BLA-BLA
   	    {
   	    	// !!!!! �������� ����������
   	    }
   	    else if (stristr(state_of_sim800_num1.rec_SMS_data,"helo-helo-helo")) //
   	    {
   	    	// !!!!! �������� ����������
   	    }
   	    else
   	    {
   	        return; // �� ��� ��������� SMS-��������� �� �������� ��������
   	    }
   	    recSMSstatus = free;
    }
}

// ������� ���������������� ������� GSM
// ����� ���������� �� ����������� ���������� (�������� �������)
// ��� �� ������ �� ������� ������������ ������� (�������� FreeRTOS, �� �� �������� ����)
void GSM_Communication_routine(void)
{
	// ��������� ���������� �����
	static uint8_t cur_dig_input;
    static GSM_counter; // ������� �������� ������ ���������������� ������� GSM

    GSM_counter++; // ����� ��������� ����������� ���������� ������� ��� ���������� �������� ��������� �
    if (GSM_counter < 300) // ��� �� ������ ������� �� ���� �� ������
    {
    	return;
    }
    GSM_counter = 0; // ���������� ������� �������� ������

    if (state_of_sim800_num1.Status == not_ready) // ���� ������������� ��� ������ �� ����� �� ���� �� ������
	{
		return;
	}

//	if ((GSM_com_state.Status_of_ == free) && (GSM_com_state.sim800module->num_of_sms != 0)) // ���� ���� ������������� SMS
//	{
//		recSMS();
//		return;
//	}

	if (GSM_com_state.Status_of_mailing == busy) // ���� ��������� ���-��, ��� ������� �������� SMS-��������� ��� GSM-������ ����� ���������� ����������� �������
    {

		sendSMS();
    	return;
    }

//    if (GSM_com_state.Status_of_mailing == free) // ���� �������� ����������� SMS-��������� � ��������� ������� �� ������ SMS-��������� ���������
//    {
//    	if ((reg74hc165_current_state_num1.arr_res[cur_dig_input].status.cur_log_state == 1) &&  // ���� �� ����� �� ������ ��������� �������� ���������
//    		(reg74hc165_current_state_num1.arr_res[cur_dig_input].status.already_sent == 0))     // � �� ���� ��� �� ��������� SMS-���������
//    	{
//    		GSM_com_state.Status_of_mailing = busy; // �������� ��������
//    		reg74hc165_current_state_num1.arr_res[cur_dig_input].status.already_sent = 1;   // �������� �������������� ����� ��� ������������ �� �������� (��� �� SMS ����������� ���� ���)
//    		FLASH_Read_Msg_String(cur_dig_input, GSM_com_state.SMS_text, MAX_SIZE_STRING_8);
//
//    	}
//    	else if (reg74hc165_current_state_num1.arr_res[cur_dig_input].status.cur_log_state == 0) // ���� �� ����� ��� ��������� ���������
//    	{
//    		reg74hc165_current_state_num1.arr_res[cur_dig_input].status.already_sent = 0; // ���������� ������� ������������ ��������� � ����
//    	}
//    	cur_dig_input++;
//    	if (cur_dig_input == NUM_OF_INPUT)
//    	{
//    		cur_dig_input = 0;
//    	}
//    	//��������� ����� ���
//    }

}
