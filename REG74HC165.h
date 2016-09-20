// ������� ��������� 74HC165 � ������������ ������ � ���������������� ������� ���������� ��������
#ifndef __REG74HC165_H_
#define __REG74HC165_H_

#define   NUM_OF_74HC165 3 // ����� ��������������� ���������

#define   REG74HC165_PORT GPIOC        // ���� �� ������� ����� �������� 74hc165

#define   CP_PIN          GPIO_Pin_14  // ����� - ����� ������
#define   CP_PIN_DOWN GPIOC->ODR |=  GPIO_Pin_14
#define   CP_PIN_UP GPIOC->ODR   &= ~GPIO_Pin_14
#define   CP_PIN_STATE  GPIOC->ODR & GPIO_Pin_14

#define   PL_PIN          GPIO_Pin_13  //������������ ������
#define   PL_PIN_DOWN GPIOC->ODR |=  GPIO_Pin_13
#define   PL_PIN_UP GPIOC->ODR   &= ~GPIO_Pin_13
#define   PL_PIN_STATE  GPIOC->ODR & GPIO_Pin_13

#define   QH_PIN          GPIO_Pin_15  //���� ������
#define   QH_PIN_DOWN GPIOC->ODR |=  GPIO_Pin_15
#define   QH_PIN_UP GPIOC->ODR   &= ~GPIO_Pin_15
#define   QH_PIN_STATE  GPIOC->ODR & GPIO_Pin_15

#define   NUM_BIT         (8*NUM_OF_74HC165) //���������� ������-��� ...1 ������� 8 ������

// ���������� ��������� ������� �������� ������ ������ �� ��������� 74HC165
// ��������� ��������� ������� ��������� �������� ��������� ������ ������ �� ���������� ��������������� ����������� ���������������� ��������� 74hc165
struct reg74hc165_current_state{
	uint8_t current_bit;                       // ������� ����������� ���
	uint32_t input_data;                       // ����������� ��������
	uint32_t result;                           // ���������
};

extern struct reg74hc165_current_state reg74hc165_current_state_num1; // ����������� �������� ����� ���� ���������

void load_data74HC165(struct reg74hc165_current_state * current_state); // ������� ���������� �� ����������� ���������� ������� ������������ �������� ������ ������ � ������ 74hc165

#endif
