// ������� ��������� 74HC165 � ������������ ������ � ���������������� ������� ���������� ��������
#ifndef __REG74HC165_H_
#define __REG74HC165_H_

#define   NUM_OF_74HC165 3 // ����� ��������������� ���������

#define   REG74HC165_PORT GPIOC        // ���� �� ������� ����� �������� 74hc165

#define   CP_PIN          GPIO_Pin_14  // ����� - ����� ������
#define   CP_PIN_UP GPIOC->ODR |=  GPIO_Pin_14
#define   CP_PIN_DOWN GPIOC->ODR   &= ~GPIO_Pin_14
#define   CP_PIN_STATE  GPIOC->IDR & GPIO_Pin_14

#define   PL_PIN          GPIO_Pin_13  //������������ ������
#define   PL_PIN_UP GPIOC->ODR |=  GPIO_Pin_13
#define   PL_PIN_DOWN GPIOC->ODR   &= ~GPIO_Pin_13
#define   PL_PIN_STATE  GPIOC->IDR & GPIO_Pin_13

#define   QH_PIN          GPIO_Pin_15  //���� ������
#define   QH_PIN_UP GPIOC->ODR |=  GPIO_Pin_15
#define   QH_PIN_DOWN GPIOC->ODR   &= ~GPIO_Pin_15
#define   QH_PIN_STATE  GPIOC->IDR & GPIO_Pin_15

#define   BIT_23  (1<<22)

#define   TRUE  1
#define   FALSE 0

#define   NUM_OF_INPUT         (8*NUM_OF_74HC165) //���������� ������-��� ...1 ������� 8 ������

// ������ �������� ������ ��� �� �������� 74hc165
// ���� �������� 74hc165 ����������� �� ������, ����� ���������� ������� ���������, ���� �� ������ �������� ��������� �� ������
enum r74hc165_stage {
	pl_low,         //����� ����, ��� ��������� NUM_BIT ��� ��� ������ �������, ���� ���������� ��������� ������
	cp_low,         //CP - � ����
    cp_high,        //CP - � �������, �� �� ������ QH ��� ������ ������ ������
    qh_ready,       //QH - �����, ������ ������
};

// ������� ���� ������� ��������� ������ ��������� �����
struct diginput_status
{
   unsigned enable         : 1; // ����� ��������� FALSE - �������� (������������� ��� ������ ��������� ������), ���� TRUE - ��������
   unsigned cur_phis_state : 1; // ������� ���������� ���������: 1 - � ������� (+�������)
                                //                               0 - � ���� (�����)
   unsigned alarm_state    : 1; // ��� ������� ���������� ����������: 1 - +������� - �������� ���������
                                //                                    0 - ���� (�����) - �������� ���������
   unsigned cur_log_state  : 1; // ������� ���������� ���������: 1 - ��������
                                //                               0 - ����������
                                // ���������� ����� ����� ����������� ������� cur_phis_state � ����, ��� ������� �������� ���������� alarm_state
   unsigned already_sent   : 1; // ������ �������� ��������� (������� SMS) � ��������� ��������� ����������� ��������� (�������� ���������� SMS)
                                // ���� ���������� ��������� ���������������� ����� ��������� (cur_log_state = 0), �� ��� ���� 0,
                                // ���� ���, �� �� �������� SMS ����� 0, ����� �������� SMS ����� 1 (��� �� SMS �� ������������ ��� ���������)
   unsigned reserved       : 3; // ��������� 3 ���� ���� � �������
};

struct digital_input
{
	struct diginput_status status;            // ������� ��������� �����
	uint32_t               str_flash_page;    // �������� ����� ������ ��� �������� ������ ��������� ��� �������� ��������� ����� (������������� ��� �����)
	uint8_t                flash_string_cell; // ���������� ������ ������ �������� ���� ������
};

// ��������� ��������� ������� ��������� ����������� �������� ������
struct reg74hc165_current_state{
	enum r74hc165_stage  stage;                // ������ ������
	uint8_t              current_bit;          // ������� ����������� ���
	struct digital_input arr_res[NUM_OF_INPUT];     // ������ ������ ���������� ��� ���������� � ���
};

extern struct reg74hc165_current_state reg74hc165_current_state_num1; // ����������� �������� ����� ���� ���������

void load_data74HC165(struct reg74hc165_current_state * current_state); // ������� ���������� �� ����������� ���������� ������� ������������ �������� ������ ������ � ������ 74hc165
uint8_t diginputsconfig(struct digital_input * arr_res); //������� ���������������� �������� ������ ��� ��������� ������� ��� �������������������� ������������� ��������� �������� ��������� � ���������

#endif
