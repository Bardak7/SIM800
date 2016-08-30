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

// ������ �������� ������� � ������� SIM800
enum com_stage {
	req_sent,         //������ ���������, �� ����� ��� �� �������
	resp_rec,         //����� �������, �� ��� �� ���������
	proc_completed    //��������� ������ �� ������ ���������
};

// ���������� ��������� ������� �������� ������ ������� � SIM800
// ��������� ��������� ������� ��������� �������� ������ �������� � ��������� ������� �� ����������� ������ SIM800 (�� ����� ���� ���������)
typedef struct {
    char current_cmd[CURRENT_CMD_SIZE];       // ������ ��� ������� �������������� SIM800 ������� (��������� ��� �������� ���)
    com_stage communication_stage;            // ������� ��������� �������� ��������� �������
    uint8_t current_pos;                      // ������� ������� ���������� ��������� ������� � �������� ������
    //uint8_t is_responce;                      // ���� ��� ����, ��� �� (���� ������� ����� ��� � SIM800) �������� ��������� ��������� �� ������� �� �������� �������
    uint8_t rec_buf[REC_BUF_SIZE];            // �������� �����, ���� ����� ������������ ����� �� SIM800 (����������� ������������ ����������)
    uint8_t responce[REC_BUF_SIZE];           // ����� � �������������� ������� �� SIM800 ��� ��������� ������ �� ��������� ������ ��� ���������� ���������
    //uint8_t unexp_responce[REC_BUF_SIZE];     // ����� � �������������� ������� �� SIM800 ��� ��������� ���������� ���������
    void (*response_handler)(void);           // ��������� �� ���������� ������
    void (*send_uart_function)(char *);       // ��������� �� ������� �������� ������ � ���������� UART �� ������� ����� ���������� ������ SIM800
    uint8_t result_of_last_execution;         // ��������� ���������� ��������� ������� 0 - OK, 1 - fail
    uint8_t num_of_sms;                       // ����� ��������� ��� ���������� � SIM-�����
} sim800_current_state;

extern sim800_current_state sim800_1_current_state; // ������� ����� ���� ���������

void sim800_PWRKEY_on(void); // ������� ��������� ������ SIM800
//!!!!!!inline void Sim800_WriteCmd(const char *cmd); // ������� �������� ������ � UART �� ������� ����� Sim800
void sim800_AT_request(sim800_current_state * current_module); // ������� �������� ������� �� ������������� baudrate ������ SIM800
void sim800_AT_responce_handler(uint8_t * responce, uint8_t result, uint8_t is_busy); // ���������� ������ ������� "AT"
uint8_t sim800_init(sim800_current_state * current_module, int32_t * init_data); // ������� ������������� ������ �� ������� SIM800
int8_t sim800_request(sim800_current_state * current_state); // ������� �������� �������� � SIM800
void process_echo(uint8_t is_responce, uint8_t current_pos, sim800_current_state *current_state); // ��������� ���
void process_cmd(uint8_t is_responce, uint8_t current_pos, sim800_current_state *current_state);  // ��������� ������ �� �������
void sim800_response_handler(sim800_current_state * current_state, uint8_t data); // ������� ���������� �� ����������� ���������� �� ������ �������� �� SIM800
void unexpec_message_parse(uint8_t * unexp_responce); //������� �������� ��������� ��������� �� SIM800 (�������� ������ SMS)
//uint8_t sim800_sendSMS(uint8_t* text_buf, uint8_t length);   // ������� �������� SMS � ������ SIM800

#endif
