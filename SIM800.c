#include <stdio.h>
#include "phisic.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "SIM800.h"



struct sim800_current_state state_of_sim800_num1; // ������� ����� ���� ���������

//const struct request AT_req = {"AT", sim800_AT_responce_handler}; // ������� AT �������� �����
//const struct request ATplusCMGS_req = {"AT+CMGS=", sim800_ATplusCMGS_responce_handler}; // ������� AT+CMGS=�����������ջ - �������� ���

// ������� ��������� ������� ������ SIM800
void sim800_1_PWRKEY_on(void)
{
    volatile int i;
    // ������ ������� �� PowerKey � ������� 3-4 ������ (�������� SIM800)
    PWR_KEY_DOWN;
    for(i=0;i<0x500000;i++);
    PWR_KEY_UP;
    for(i=0;i<0x500000;i++);
    //	// ������ �� ���� PWRKEY ���. 0
    //    GPIO_WriteBit (GPIO_SIM800_PWRKEY, SIM800_PWRKEY, Bit_RESET);
    //    // �������� � ��., ����������� ��� ���������
    //    for (i = 0; i<5000; i++);
    //
    //    // ������ �� ���� PWRKEY ���. 1
    //    GPIO_WriteBit (GPIO_SIM800_PWRKEY, SIM800_PWRKEY, Bit_SET);
    //    // ������� ���������/���������� SIM800
    //    for (i = 0; i<5000; i++);
}

// ������� �������� ������ � UART �� ������� ����� Sim800
inline void Sim800_WriteCmd(struct sim800_current_state *current_state)
{


    current_state->send_uart_function(current_state->current_cmd);
}

// ������� �������� ��� � UART �� ������� ����� Sim800
inline void Sim800_WriteSMS(struct sim800_current_state *current_state)
{
    current_state->send_uart_function(current_state->send_SMS_data);
    current_state->send_uart_function("\032"); // ��������� ��� ��������� ��������� ctrl+Z
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
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    current_state->communication_stage = resp_rec; // ���������� ���� ���������
    Sim800_WriteCmd(current_state);



    return exit_and_wait;                         // ������������ � ��������� ��� ������ ����� ������ � ���������� ������� ������, ������ ���������
}

//**********************************************************************
//**********************************************************************
//������� �������� �������� � ����������� ������� (������� ������/�����)

// ������� �������� ������� �� ������������� baudrate ������ SIM800 (������� "AT")
uint8_t sim800_AT_request(struct sim800_current_state * current_state)
{
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    memcpy(current_state->current_cmd, "AT\r", 4);
    current_state->response_handler = sim800_AT_responce_handler;
    return sim800_request(current_state);
}

// ���������� ������ ������� "AT"
void sim800_AT_responce_handler(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"AT")) // ������ ���?
    {
        //int j; GPIOA->ODR &= ~GPIO_Pin_0; for(j=0;j<0x50000;j++); GPIOA->ODR |= GPIO_Pin_0; // �������!!!
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

// ������� ������������ SIM800 � ��������� ����� (������� "AT+CMGF=1 ��� 0 1-��������, 0-���������")
// ���������: 1) ��������� �� ��������� ��������� ������� ������ SIM800
//            2) ����� ����� ������ text_mode = 1 - �������� ��������� �����, code_mode = 0 - ��������� ��������� �����
// ���������� ������ ����������
uint8_t sim800_ATplusCMGF_request(struct sim800_current_state * current_state, uint8_t mode)
{
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    memcpy(current_state->current_cmd, "AT+CMGF=", 9);

    uint8_t string_of_mode[2];
    itoa(mode, string_of_mode, 10);

    strncat(current_state->current_cmd, string_of_mode, 1);
    //    if (mode == text_mode)
    //    {
    //    	strncat(current_state->current_cmd, "1", 1);
    //    }
    //    else
    //    {
    //    	strncat(current_state->current_cmd, "0", 1);
    //	}

    strncat(current_state->current_cmd, "\r", 2);
    current_state->response_handler = sim800_ATplusCMGF_responce_handler;
    return sim800_request(current_state);
}

// ���������� ������ ������� "AT+CMGF=0/1"
void sim800_ATplusCMGF_responce_handler(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"AT+CMGF=")) // ������ ���?
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
        current_state->unex_resp_handler(current_state); // ���� ������ �� ����� �� ���� �������, � ���-�� ���, �������� ���������� ��������� ���������
        return;
    }
}

// ������� �������� ��� SIM800 (������� "AT+CMGS=�����������ջ")
// ���������: 1) ��������� �� ��������� ��������� ������� ������ SIM800
//            2) ��������� �� ������ ���������� ���������� ����� ��������
//            3) ��������� �� ������ ���������� ��������� ���������
// ���������� ������ ����������
uint8_t sim800_ATplusCMGS_request(struct sim800_current_state * current_state, uint8_t * phone_number, uint8_t * SMS_data)
{
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    memcpy(current_state->current_cmd, "AT+CMGS=", 9);
    strncat(current_state->current_cmd, "\"", 2);
    strncat(current_state->current_cmd, phone_number, 14); // 14 - ������� � ������� �� ������ �������� (������� ���������� ����� �������� 11 �������)
    strncat(current_state->current_cmd, "\"", 2); // ������� ������� � ����� � ������ ����������� ������ ��� ���� ������� �������� AT-�������
    strncat(current_state->current_cmd, "\r", 2);
    current_state->response_handler = sim800_ATplusCMGS_responce_handler;

    memcpy(current_state->send_SMS_data, SMS_data, SEND_SMS_DATA_SIZE);
    strncat(current_state->send_SMS_data, 0x1A, 1); // � ����� ��� ������ ����������� �������� Ctrl+Z (������� ��������)
    strncat(current_state->send_SMS_data, "\0", 1);
    return sim800_request(current_state);
}

// ���������� ������ ������� "AT+CMGS=�����������ջ"
void sim800_ATplusCMGS_responce_handler(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"AT+CMGS=")) // ������ ���?
    {
        return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
    }
    else if (strchr(current_state->responce, '>')) // ������ ����������� ������ ����� ��� ���������
    {
        Sim800_WriteSMS(current_state); // ���� ���-��������� ����� ������ current_state � ������� Sim800_WriteSMS �������� ��� �� ����
        return;
    }
    else if (strstr(current_state->responce,"+CMGS:"))
    {
        //��������� ���������� ��������� �� �������� SMS ����:
        //+CMGS: XXX (XXX - ����� �������� ����� ������������� SMS)
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

// ������� �������� ��� SIM800 (������� "AT+CMGD=1,0" 1, � ����� ��������� 0, � ����� ��������)
// ���������: 1) ��������� �� ��������� ��������� ������� ������ SIM800
//            2) ����� ���������
//            3) ����� ��������
// ���������� ������ ����������
uint8_t sim800_ATplusCMGD_request(struct sim800_current_state * current_state, uint8_t num_of_message, uint8_t mode_of_delete)
{
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    memcpy(current_state->current_cmd, "AT+CMGD=", 9);

    uint8_t buf_string[3]; // � ������� SIM-����� ���������� �� ����� 10 - 20 ��� ���������
    itoa(num_of_message, buf_string, 10);
    strncat(current_state->current_cmd, buf_string, 3);

    strncat(current_state->current_cmd, ",", 2); // ������� ������� ��� ���� ������� ������ �������� �������

    // ������� ����� ���� ����� 5: 0 � �������� ���������� ���������. �������� �� ���������, ����� ������������ ������ AT+CMGD=2
    //                             1 � �������� ������ ���� ����������� ���������
    //                             2 � �������� ����������� � ������������ ���������
    //                             3 � �������� ���� �����������, ������������ � �� ������������ ���������
    //                             4 � �������� ���� ���������

    itoa(mode_of_delete, buf_string, 10);
    strncat(current_state->current_cmd, buf_string, 2);

    strncat(current_state->current_cmd, "\r", 2);
    current_state->response_handler = sim800_ATplusCMGD_responce_handler;
    return sim800_request(current_state);
}

// ���������� ������ ������� "AT+CMGD=
void sim800_ATplusCMGD_responce_handler(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"AT+CMGD=")) // ������ ���?
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


// ������� �������� ������� �� ������ ��� SIM800 (������� "AT+CMGR=1,0" 1, � ����� ���
//                                                                      0, � ������� ����� ��� 1, � �� �������� ������.)
// ���������: 1) ��������� �� ��������� ��������� ������� ������ SIM800
//            2) ����� ���������
//            3) ����� ������
// ���������� ������ ����������
uint8_t sim800_ATplusCMGR_request(struct sim800_current_state * current_state, uint8_t num_of_message, uint8_t mode_of_read)
{
    if(current_state->communication_stage != proc_completed) // ������ �� ������� ������ ��������
    {
        return busy;                             // ������ ��������� �� �������, �.�. ���������� ������ ��� �� ������� ��� �� ���������
    }
    memcpy(current_state->current_cmd, "AT+CMGR=", 9);

    uint8_t buf_string[3]; // � ������� SIM-����� ���������� �� ����� 10 - 20 ��� ���������
    itoa(num_of_message, buf_string, 10);
    strncat(current_state->current_cmd, buf_string, 3);

    strncat(current_state->current_cmd, ",", 2); // ������� ������� ��� ���� ������� ������ �������� �������

    // ������� ����� ���� ����� 2: 0 � ������� �����
    //                             1 � �� �������� ������

    itoa(mode_of_read, buf_string, 10);
    strncat(current_state->current_cmd, buf_string, 2);

    strncat(current_state->current_cmd, "\r", 2);
    current_state->response_handler = sim800_ATplusCMGR_responce_handler_st1;
    return sim800_request(current_state);
}

// �������� ��������� ������ ������� �� ������, �.�. ����� ������� � ��� ������� �� ������� �� ������� ���������� OK
// ���������� ������ ������� "AT+CMGR= - ������ SMS ������ 1 - ����� ���
void sim800_ATplusCMGR_responce_handler_st1(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"AT+CMGR=")) // ������ ���?
    {
        current_state->response_handler = sim800_ATplusCMGR_responce_handler_st2;
        return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
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

// ���������� ������ ������� "AT+CMGR= - ������ SMS ������ 2 - ����� ��������� ������ � SMS
void sim800_ATplusCMGR_responce_handler_st2(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"+CMGR:"))
    {
        //GPIOA->ODR &= ~GPIO_Pin_0; // �������!!!
        current_state->response_handler = sim800_ATplusCMGR_responce_handler_st3;
        // ���� �� ���� �� ������, �� �� ������ ����
        // +CMGR: "REC UNREAD","+7XXXXXXXXXX","","16/09/06,14:17:35+12" ����� ������� ����� �������� ����������
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

// ���������� ������ ������� "AT+CMGR= - ������ SMS ������ 3 - ����� ������ ������ SMS
void sim800_ATplusCMGR_responce_handler_st3(struct sim800_current_state * current_state)
{
    //			if (strstr(current_state->responce,"REC"))
    //			{
    //				GPIOA->ODR &= ~GPIO_Pin_0; // �������!!!
    //			}
    current_state->response_handler = sim800_ATplusCMGR_responce_handler_st4;
    memcpy(current_state->rec_SMS_data, current_state->responce, strlen(current_state->responce)); // �������� �������� SMS ���������
    return;
}

// ���������� ������ ������� "AT+CMGR= - ������ SMS ������ 4 - ��������� ��������� OK
void sim800_ATplusCMGR_responce_handler_st4(struct sim800_current_state * current_state)
{
    if (strstr(current_state->responce,"OK"))
    {
        //int j; GPIOA->ODR &= ~GPIO_Pin_0; //for(j=0;j<0x50000;j++); GPIOA->ODR |= GPIO_Pin_0; // �������!!!
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
    current_state->communication_stage = proc_completed; // � ������ ������ ���������� ������� ������ � ����
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
    current_state->PWR_KEY_handler = sim800_1_PWRKEY_on;    // ��������� �� ������� ��������� ����������� ������ SIM800
    current_state->PWR_KEY_handler();                       // �������� ������
    current_state->is_Call_Ready = not_ready;
    current_state->is_SMS_Ready = not_ready;

    // ������� ��������� �������� �������, ������ �������� �������� �������� �� UART
    sim800_AT_request(&state_of_sim800_num1);








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

// ��������������� ������� ����������� ����������� ��������� ������ � ����� ��������� ������
// � ������ ���������������� ����������� ��������� ������
void call_handler(struct sim800_current_state * current_state)
{

    if (current_state->response_handler != NULL)
    {
        current_state->communication_stage = resp_rec; // ����� �������
        current_state->response_handler(current_state);
    }
    else // ���� �� ��������� �� ���������� ������ ����� NULL, ������ ������ �� ����������� � ��� ����������� ��������� �� SIM800 (�������� ������ ���)
    {
        current_state->unex_resp_handler(current_state);
    }
    return;
}

// ������� ���������� �� ����������� ���������� �� ������ �������� �� SIM800
// ������ ��������� ������ ������ � ���������� ��� �� ���������� ������ � ���������
// ���������: ��������� �� ��������� ���������� ������� ��������� ����������� ������ SIM800(�� ����� ���� ���������) � �������� �� UART ������
// �� ���� �� ����������, � � ���� ���������� ����� � �������� ����� � � ������ ������ ���� ������� �������� �������������� ���������� ������
void sim800_response_handler(struct sim800_current_state * current_state, uint8_t data)
{
    current_state->rec_buf[current_state->current_pos++] = data;
    current_state->rec_buf[current_state->current_pos] = '\0'; // ��������� ��������� �� ��������� �������� �������� ������� ����-������������ ��� ���������� ������ ������� ��������� strstr, memcpy � ������
    if (current_state->current_pos > REC_BUF_SIZE - 1)
    {
        // � ������ ������������ ����������� ���������� �����
        current_state->current_pos = 0;
        return;
    }

    //���� ��������� ����������� � ����� SMS-�� (������ '>')
    if (strchr(&current_state->rec_buf[current_state->current_pos - 1], '>'))
    {
        memcpy(current_state->responce, current_state->rec_buf, current_state->current_pos + 1); // �������� ���������� ��������� ������ � ����� ��������� ������ (�.�. ���������� ��������� ������ ���������� ���������� ��� ������)
        current_state->current_pos = 0;
        call_handler(current_state);
        return;
    }

    if (current_state->current_pos <= 3) // ���� ������� ������ 3-� �������� ������ �� ��������� �������� ������ ������������
        return;

    // ���� ��������� �������� ����� �� ������� �������� ����� ��������� ��������� (��� ���������������� ������� "\r\n")
    if( strstr(&current_state->rec_buf[current_state->current_pos - 2], "\r\n") )
    {
        memcpy(current_state->responce, current_state->rec_buf, current_state->current_pos + 1); // �������� ���������� ��������� ������ � ����� ��������� ������ (�.�. ���������� ��������� ������ ���������� ���������� ��� ������)
        current_state->current_pos = 0;
        call_handler(current_state);
        return;
    }
    //    if( strstr(&current_state->rec_buf[current_state->current_pos - 2], "OK") )
    //    {
    //    	call_handler(current_state);
    //    	return;
    //    }
    //    if( strchr(&current_state->rec_buf[current_state->current_pos - 1], '\r') )
    //    {
    //    	call_handler(current_state);
    //    	return;
    //    }
}

//������� �������� ��������� ��������� �� SIM800 (�������� ������ SMS)
void unexpec_message_parse(struct sim800_current_state *current_state)
{
    //if (strstr(current_state->responce,"+CMGS:")) // ��� ������������
    //{
    //    return; // �� ���� �� ������ (���� ����� ����� ������� ����� ����)
    //}
    if (strstr(current_state->responce,"+CMTI:")) // ������ ��� ��������� (������� "+CMTI: "SM",12")
    {
        // !!!!!!!!! ��� ���� ������� ��������� ��������� ��� ��������� ��� ������ ���������������� ������� �������� ���������
        int j; GPIOA->ODR &= ~GPIO_Pin_0; //for(j=0;j<0x50000;j++); GPIOA->ODR |= GPIO_Pin_0; // �������!!!
        return;
    }
    else if (strstr(current_state->responce,"Call Ready")) //
    {
        current_state->is_Call_Ready = ready;
        int j; GPIOA->ODR &= ~GPIO_Pin_0; for(j=0;j<0x50000;j++); GPIOA->ODR |= GPIO_Pin_0; // �������!!!
        return;
    }
    else if (strstr(current_state->responce,"SMS Ready")) //
    {
        current_state->is_SMS_Ready = ready;
        int j; GPIOA->ODR &= ~GPIO_Pin_0; for(j=0;j<0x50000;j++); GPIOA->ODR |= GPIO_Pin_0; // �������!!!
        return;
    }
    else if (strstr(current_state->responce,"RING")) // ��� ������.
    {
        // ���� �� ���� �� ������
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
