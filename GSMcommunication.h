// ������� ��� �������� � ������ ������ �� ���������� GSM

#ifndef __GSM_COM_H_
#define __GSM_COM_H_

#define busy 1
#define free 0

#define SMS_send_start  1 // ������� ������ �������� SMS
#define SMS_send_stop   0 // ������� ����� �������� SMS(����� GSM ������ �������, ��� SMS ����� ����)

#define NUM_OF_ABONENTS 5 // NUM_OF_ABONENTS ������ ���� ������ MAX_NUM_OF_ABONENTS = 32

void GSM_Com_Init(struct sim800_current_state * current_state); // ������� ������������� ����������������� ����������

void GSM_Communication_routine(void); // ������� ���������������� ������� GSM

#endif
