#ifndef __SIM800_H_
#define __SIM800_H_

#define busy 1
#define free 0

#define yes 1
#define no  0

#define CURRENT_CMD_SIZE 16 // ������ ������ ��� ������������ SIM800 �������
#define REC_BUF_SIZE 16 // ������ ������ ��� ����������� �� SIM800 ������ (��� ������ ��������� ������� ������� ������ ��������� ��������� ��� �����)

int8_t sim800_routine(const char *cmd, uint8_t rec_uart_char, const uint8_t response_data,  void (*callback)(void));

#endif
