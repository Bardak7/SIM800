#ifndef __SIM800_H_
#define __SIM800_H_

#define busy 1
#define free 0

#define OK   0
#define fail 1

#define yes  1
#define no   0

#define exit_and_wait 0

#define CURRENT_CMD_SIZE 16 // ������ ������ ��� ������������ SIM800 �������
#define REC_BUF_SIZE 256    // ������ ������ ��� ����������� �� SIM800 ������ (��� ������ ��������� ������� ������� ������ ��������� ��������� ��� �����)
#define DATA_BUF_SIZE 256   // ������ ������ ��� ������������ � SIM800 ������ (��� ������ �������� ������� ������� ������ ��������� ��������� ��� �����)
#define SEND_SMS_DATA_SIZE 256   // ������ ������ ��� ������������ ��� ���������
#define REC_SMS_DATA_SIZE 256    // ������ ������ ��� ����������� ��� ���������
#define PHONE_NUM_SIZE 16        // ������ ������ ��� ���������� ����� ������ � ������� (���������� 11 ��������, �� �������� �� ������� ������)

// ������ �������� ������� � ������� SIM800
enum com_stage {
	req_sent,         //������ ���������, �� ����� ��� �� �������
	resp_rec,         //����� �������, �� ��� �� ���������
	proc_completed    //��������� ������ �� ������ ���������
};

//// ��������� �������������� �� ���� ������������ AT-�������
//struct request {
//	char current_cmd[CURRENT_CMD_SIZE]; // ���� �������
//	void (*response_handler)(uint8_t * responce, uint8_t * result_of_last_execution, enum com_stage communication_stage, void (*unex_resp_handler)(uint8_t * responce), uint8_t num_of_sms;);
//	// ��������� �� ���������� ������ ��� ������ �������
//	// ������ ���������:1) ��������� �� ����� ��� ����� �������������� � ���������� �����
//	//                  2) ��������� �� �� ���� ������� ��������� ��������� ������
//	//                  3) ��������� �� ������ ������ ��� ����� ���� �������� ��������� �������� ��������� �������. ��� ���� ���-�� ���������� ������� ���� proc_completed - ��������� ������ �� ������ ���������
//    //                  4) ��������� �� ���������� ����������� �������, ���� ������ ������� ����������, ��� ����� ��������� �� ��
//};

// ���������� ��������� ������� �������� ������ ������� � SIM800
// ��������� ��������� ������� ��������� �������� ������ �������� � ��������� ������� �� ����������� ������ SIM800 (�� ����� ���� ���������)
struct sim800_current_state{
    enum com_stage communication_stage;             // ������� ��������� �������� ��������� �������
    uint8_t current_pos;                            // ������� ������� ���������� ��������� ������� � �������� ������
    uint8_t rec_buf[REC_BUF_SIZE];                  // �������� �����, ���� ����� ������������ ����� �� SIM800 (����������� ������������ ����������)
    uint8_t responce[REC_BUF_SIZE];                 // ����� � �������������� ������� �� SIM800 ��� ��������� ������ �� ��������� ������ ��� ���������� ���������
    //struct request *current_req;                  // ��������� �� ������� �������������� �������
    void (*send_uart_function)(char *);             // ��������� �� ������� �������� ������ � ���������� UART �� ������� ����� ���������� ������ SIM800
    uint8_t result_of_last_execution;               // ��������� ���������� ��������� ������� 0 - OK, 1 - fail
    uint8_t num_of_sms;                             // ����� ��������� ��� ���������� � SIM-�����
    void (*unex_resp_handler)(struct sim800_current_state *current_state); // ��������� �� ���������� ������������ ������
    char current_cmd[CURRENT_CMD_SIZE];             // ������������ �������
    void (*response_handler)(struct sim800_current_state *current_state); // ��������� �� ���������� ������ ��� ������� �������������� �������
    uint8_t send_phone_number[PHONE_NUM_SIZE];      // ������� ����� �������� ��� ������������ ��� ���������
    uint8_t rec_phone_number[PHONE_NUM_SIZE];       // ������� ����� �������� ��������� ��� ���������
    uint8_t send_SMS_data[SEND_SMS_DATA_SIZE];      // ����� ��� ������������ ��� ���������
    uint8_t rec_SMS_data[REC_SMS_DATA_SIZE];        // ����� ��� ����������� ��� ���������
};

extern struct sim800_current_state state_of_sim800_num1; // ������� ����� ���� ���������

void sim800_PWRKEY_on(void); // ������� ��������� ������ SIM800
//!!!!!!inline void Sim800_WriteCmd(const char *cmd); // ������� �������� ������ � UART �� ������� ����� Sim800
//void sim800_AT_request(struct sim800_current_state *current_state); // ������� �������� ������� �� ������������� baudrate ������ SIM800

uint8_t sim800_init(struct sim800_current_state * current_state, void (*send_uart_function)(char *)); // ������� ������������� ������ �� ������� SIM800
int8_t sim800_request(struct sim800_current_state *current_state); // ������� �������� �������� � SIM800
void process_echo(uint8_t is_responce, uint8_t current_pos, struct sim800_current_state *current_state); // ��������� ���
void process_cmd(uint8_t is_responce, uint8_t current_pos, struct sim800_current_state *current_state);  // ��������� ������ �� �������
void sim800_response_handler(struct sim800_current_state *current_state, uint8_t data); // ������� ���������� �� ����������� ���������� �� ������ �������� �� SIM800


void sim800_AT_request(struct sim800_current_state * current_state); // ������� �������� ������� �� ������������� baudrate ������ SIM800 (������� "AT")
void sim800_AT_responce_handler(struct sim800_current_state * current_state); // ���������� ������ ������� "AT"

void sim800_ATplusCMGS_request(struct sim800_current_state * current_state, uint8_t * phone_number, uint8_t * SMS_data); // ������� �������� ��� SIM800 (������� "AT+CMGS=�����������ջ")
void sim800_ATplusCMGS_responce_handler(struct sim800_current_state * current_state); // ���������� ������ ������� "AT+CMGS=�����������ջ"

void unexpec_message_parse(struct sim800_current_state *current_state); //������� �������� ��������� ��������� �� SIM800 (�������� ������ SMS)
//uint8_t sim800_sendSMS(uint8_t* text_buf, uint8_t length);   // ������� �������� SMS � ������ SIM800

#endif
