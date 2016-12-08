﻿#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "flash.h"
//#include "SIM800.h"  // КОСТЫЛЬ!!!
#include "REG74HC165.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_flash.h"
#include "core_cmFunc.h"
#include "phisic.h"
//#include "GSMcommunication.h" // КОСТЫЛЬ!!!

const uint8_t  std_string_prefix1[] = "PREALARM ON INPUT "; // префикс строк текстовых сообщений, записываемых по умолчанию если соответствующая строка сообщения в ячеке-строек пуста
const uint8_t  std_string_prefix2[] = "ALARM ON INPUT "; // префикс строк текстовых сообщений, записываемых по умолчанию если соответствующая строка сообщения в ячеке-строек пуста
const uint32_t DATA_Pages[NUM_OF_STR_PAGE] = { start_DATA_Page_61 , start_DATA_Page_62 , start_DATA_Page_63 };

//***************************************************************************************************************************************
uint32_t FLASH_Read(uint32_t address) // чтение данных из флеш памяти по заданному адресу
{
    return (*(__IO uint32_t*)address);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция чтения одного байта из произвольной страницы флеш памяти по произвольному смещению
// принимает: 1) адрес страницы флеш памяти
//            2) смещение байта начиная с нуля (их в одной странице 1024-е штуки)
//            3) указатель на переменную типа uint8_t, куда будет произведена запись
uint8_t FLASH_Read_Byte(uint32_t page, uint16_t byte_shift)
{
    union
    {
        uint32_t i32;
        uint8_t  i8[4];
    } temp_buf;

    if (byte_shift >= PAGE_SIZE_8)
    {
        return -1;
    }

    // читаем ячеку из флеш
    temp_buf.i32 = FLASH_Read(page + 4*(byte_shift/4)); // сохраняем данные 32-х битной ячейки флеш памяти

    return temp_buf.i8[byte_shift%4];
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция чтения произвольной строки из флеш,
// из произвольной страницы, из произвольного адреса внутри страницы
// принимает: 1) адрес страницы флеш памяти
//            2) смещение внутри страницы кратное 4-м байтам
//            (размер страницы 1024 байта, чтение ведется по словами по 4 байта)
//            3) указатель на строку для копирования
//            4) размер копируемой строки
uint8_t FLASH_Read_String(uint32_t page, uint32_t shift, uint8_t * data_string, uint32_t size)
{
    uint32_t i;

    uint32_t temp_buf[MAX_SIZE_STRING_32];

    if (data_string == NULL)
    {
        return 1;
    }

    if ((shift%4) != 0)
    {
        return 2; // читаемые данные не выровнены на 4-байта
    }

    if ((shift + size) > PAGE_SIZE_8)
    {
        return 3;
    }

    if (size > PAGE_SIZE_8) // если размер строки окажется больше размера страницы обрезаем ее до размера страницы
    {
        size = PAGE_SIZE_8;
        data_string[PAGE_SIZE_8-1]='\0'; // и последний символ - нулевой
    }

    // читаем ячеку-строку из флеш
    for (i = 0; i < MAX_SIZE_STRING_32; i++) // чтение идет сразу по 4-е байта
    {
        temp_buf[i] = FLASH_Read(page + shift + 4*i); // сохраняем данные страницы флеш памяти
    }

    strlcpy((char*)data_string, (char*)temp_buf, size);
    return 0;

}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция чтения строки сообщения из флеш
// страница флеш памяти разбивается условно на 8-мь 128-и байтных ячеек (как раз 1024 байта - размер одной страницы)
// принимает: 1) номер ячейки под пару строк
//            2) признак (0 или 1) - это первое сообщение или второе (например об отсутствии и присутствии сигнала)
//            3) указатель на строку для копирования
//            4) размер копируемой строки
uint8_t FLASH_Read_Msg_String(uint8_t string_cell, uint8_t kind_of_msg, uint8_t * data_string, uint32_t size)
{
    uint32_t page;

    if (data_string == NULL)
    {
        return 1;
    }

    if (string_cell >= NUM_OF_INPUT_SIGNAL)
    {
        return 3;
    }

    page = start_DATA_Page_61;

    if (string_cell >= STR_CELL_IN_PAGE * 2)
    {
        string_cell -= STR_CELL_IN_PAGE * 2;
        page = start_DATA_Page_63;
    }

    if (string_cell >= STR_CELL_IN_PAGE)
    {
        string_cell -= STR_CELL_IN_PAGE;
        page = start_DATA_Page_62;
    }
    if (kind_of_msg == 0)
    {
        return FLASH_Read_String(page, MAX_SIZE_STR_CELL * string_cell, data_string, size);
    }
    if (kind_of_msg == 1)
    {
        return FLASH_Read_String(page, MAX_SIZE_STR_CELL * string_cell + MAX_SIZE_STRING_8, data_string, size);
    }
    return 4; // если пользователь указал в типе сообщения не 0 и не 1
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция чтения строки телефонного номера целевого абонента из четвертой с конца страницы флеш
// страница флеш памяти разбивается условно на 32-е 32-ух байтных строк-ячеек (как раз 1024 байта - размер одной страницы)
// принимает: 1) номер строки-ячейки (их в одной странице 32-е штуки)
//            2) указатель на записываемую строку телефонного номера
//            3) размер записываемой строки
uint8_t FLASH_Read_Phone_Num(uint8_t string_cell, uint8_t * data_string, uint32_t size)
{
    if (data_string == NULL)
    {
        return 1;
    }

    if (string_cell >= MAX_SIZE_STR_PHONE_8)
    {
        return 3;
    }

    return FLASH_Read_String(start_DATA_Page_60, MAX_SIZE_STR_PHONE_8 * string_cell, data_string, size);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция чтения байта конфигурации цифровых входов из пятой с конца страницы флеш
// под каждый вход требуется 1 байт в котором будет хранится битовое поле конфигурации
// принимает: 1) номер байта начиная с нуля (их в одной странице 1024-е штуки)
//            2) указатель на записываемую структуру конфигурации
int8_t FLASH_Read_Config_Byte(uint16_t byte_shift)
{
    if (byte_shift >= PAGE_SIZE_8)
    {
        return -1;
    }

    return FLASH_Read_Byte(start_DATA_Page_59, byte_shift);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// ВНИМАНИЕ!!! НЕВЕРНОЕ ИСПОЛЬЗОВАНИЕ ЭТОЙ ФУНКЦИИ МОЖЕТ ПОВРЕДИТЬ ПРОШИВКУ!!!
// функция записи одного байта в произвольную страницу флеш памяти по произвольному смещению
// принимает: 1) адрес страницы флеш памяти
//            2) смещение байта начиная с нуля (их в одной странице 1024-е штуки)
//            3) указатель на переменную типа uint8_t, из которой будет произведена запись
uint8_t FLASH_Write_Byte(uint32_t page, uint16_t byte_shift, uint8_t write_byte)
{
    uint32_t i;

    union
    {
        uint32_t i32;
        uint8_t  i8[4];
    } temp_buf[PAGE_SIZE_32];

    if (byte_shift >= PAGE_SIZE_8)
    {
        return 3;
    }

    // читаем страницу флеш во временный буфер
    for (i = 0; i < PAGE_SIZE_32; i++) // чтение идет сразу по 4-е байта
    {
        temp_buf[i].i32 = FLASH_Read(page + 4*i);
    }

    temp_buf[byte_shift/4].i8[byte_shift%4] = write_byte; // изменяем данные

    FLASH_ErasePage(page); // стираем страницу флеш памяти

    // записываем измененый временный буфер обратно в страницу флеш
    for (i = 0; i < PAGE_SIZE_32; i++) // запись идет сразу по 4-е байта
    {
        FLASH_ProgramWord(page + 4*i,temp_buf[i].i32);
    }

    return 0;
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// ВНИМАНИЕ!!! НЕВЕРНОЕ ИСПОЛЬЗОВАНИЕ ЭТОЙ ФУНКЦИИ МОЖЕТ ПОВРЕДИТЬ ПРОШИВКУ!!!
// функция записи произвольной строки сообщения во флеш
// в произвольную страницу, по произвольному адресу
// произвольной длины (но не больше длины страницы)
// принимает: 1) адрес страницы флеш памяти
//            2) смещение внутри страницы кратное 4-м байтам
//            (размер страницы 1024 байта, чтение ведется по словами по 4 байта)
//            3) указатель на строку для копирования
//            4) размер копируемой строки
uint8_t FLASH_Write_String(uint32_t page, uint32_t shift, uint8_t * data_string, uint32_t size)
{
    uint32_t i;

    uint32_t temp_buf[PAGE_SIZE_32]; // временный буффер в котором хранятся считанная страница из флеш памяти

    if (data_string == NULL)
    {
        return 1;
    }

    if ((shift%4) != 0)
    {
        return 2; // записываемые данные не выровнены на 4-байта
    }

    if ((shift + size) > PAGE_SIZE_8)
    {
        return 3;
    }

    if (size > PAGE_SIZE_8) // если размер строки окажется больше размера страницы
    {
        size = PAGE_SIZE_8 - 1;
    }

    // читаем страницу флеш во временный буфер
    for (i = 0; i < PAGE_SIZE_32; i++) // чтение идет сразу по 4-е байта
    {
        temp_buf[i] = FLASH_Read(page + 4*i);
    }

    memset(temp_buf + shift/4, '\0' , size + 1);
    memcpy(temp_buf + shift/4, data_string, size);

    FLASH_ErasePage(page); // стираем страницу флеш памяти

    // записываем измененый временный буфер обратно в страницу флеш
    for (i = 0; i < PAGE_SIZE_32; i++) // запись идет сразу по 4-е байта    PAGE_SIZE_32
    {
        FLASH_ProgramWord(page + 4*i,temp_buf[i]);
    }
    return 0;
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция записи строки сообщения во флеш в одну из трех последних страниц
// страница флеш памяти разбивается условно на 8-мь 128-и байтных ячеек (как раз 1024 байта - размер одной страницы)
// принимает: 1) номер ячейки под пару строк
//            2) признак (0 или 1) - это первое сообщение или второе (например об отсутствии и присутствии сигнала)
//            3) указатель на записываемую строку
//            4) размер записываемой строки
uint8_t FLASH_Write_Msg_String(uint8_t string_cell, uint8_t kind_of_msg, uint8_t * data_string, uint32_t size)
{
    uint32_t page;

    if (data_string == NULL)
    {
        return 1;
    }

    if (string_cell >= NUM_OF_INPUT_SIGNAL)
    {
        return 3;
    }

    page = start_DATA_Page_61;

    if (string_cell >= STR_CELL_IN_PAGE * 2)
    {
        string_cell -= STR_CELL_IN_PAGE * 2;
        page = start_DATA_Page_63;
    }

    if (string_cell >= STR_CELL_IN_PAGE)
    {
        string_cell -= STR_CELL_IN_PAGE;
        page = start_DATA_Page_62;
    }

    if (size > MAX_SIZE_STRING_8) // если размер строки окажется больше максимального размера строки обрезаем ее до этого размера
    {
        size = MAX_SIZE_STRING_8;
        data_string[MAX_SIZE_STRING_8-1]='\0'; // и последний символ - нулевой
    }
    if (kind_of_msg == 0)
    {
        return FLASH_Write_String(page, MAX_SIZE_STR_CELL * string_cell, data_string, size);
    }
    if (kind_of_msg == 1)
    {
        return FLASH_Write_String(page, MAX_SIZE_STR_CELL * string_cell + MAX_SIZE_STRING_8, data_string, size);
    }
    return 4; // если пользователь указал в типе сообщения не 0 и не 1
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция записи строки телефонного номера целевого абонента в четвертую с конца страницу флеш
// страница флеш памяти разбивается условно на 32-е 32-ух байтных строк-ячеек (как раз 1024 байта - размер одной страницы)
// принимает: 1) номер строки-ячейки (их в одной странице 32-е штуки)
//            2) указатель на записываемую строку телефонного номера
//            3) размер записываемой строки
uint8_t FLASH_Write_Phone_Num(uint8_t string_cell, uint8_t * data_string, uint32_t size)
{
    if (data_string == NULL)
    {
        return 1;
    }

    if (string_cell >= MAX_SIZE_STR_PHONE_8)
    {
        return 3;
    }

    if (size > MAX_SIZE_STR_PHONE_8) // если размер строки окажется больше размера ячейки-строки обрезаем ее до размера ячейки-строки
    {
        size = MAX_SIZE_STR_PHONE_8;
        data_string[MAX_SIZE_STR_PHONE_8-1]='\0'; // и последний символ - нулевой
    }

    return FLASH_Write_String(start_DATA_Page_60, MAX_SIZE_STR_PHONE_8 * string_cell, data_string, size);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция записи байта конфигурации цифровых входов в пятую с конца страницы флеш
// под каждый вход требуется 1 байт в котором будет хранится битовое поле конфигурации
// принимает: 1) номер байта начиная с нуля (их в одной странице 1024-е штуки)
//            2) указатель на записываемую структуру конфигурации
uint8_t FLASH_Write_Config_Byte(uint16_t byte_shift, uint8_t config_byte)
{
    if (byte_shift >= PAGE_SIZE_8)
    {
        return 3;
    }


    return FLASH_Write_Byte(start_DATA_Page_59, byte_shift, config_byte);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция записи всей страницы неким массивом данных
// принимает: 1) адрес страницы флеш памяти
//            2) указатель на записываемый массив структур конфигурации
//            3) размер массива
uint8_t FLASH_Write_Page(uint32_t page, uint8_t * data_array, uint8_t size)
{
    uint32_t i;

    union
    {
        uint32_t i32;
        uint8_t  i8[4];
    } temp_buf[PAGE_SIZE_32];

    if (data_array == NULL)
    {
        return 1;
    }

    if (size > PAGE_SIZE_8)
    {
        return 3;
    }

    memset(temp_buf, 0, PAGE_SIZE_8);

    memcpy(temp_buf, data_array, size);

    FLASH_ErasePage(page); // стираем страницу флеш памяти

    // записываем буфер в страницу флеш
    for (i = 0; i < PAGE_SIZE_32; i++) // запись идет сразу по 4-е байта
    {
        FLASH_ProgramWord(page + 4*i,temp_buf[i].i32);
    }

    return 0;
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция записи всей конфигурационной страницы цифровых входов в пятую с конца страницы флеш
// под каждый вход требуется 1 байт в котором будет хранится битовое поле конфигурации
// принимает: 1) указатель на записываемый массив структур конфигурации
//            2) размер массива
uint8_t FLASH_Write_Config_Page(uint8_t * config_array, uint8_t size)
{
    if (config_array == NULL)
    {
        return 1;
    }

    if (size > PAGE_SIZE_8)
    {
        return 3;
    }

    return FLASH_Write_Page(start_DATA_Page_59, config_array, size);
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция проверки строк-ячеек на заполненость
// при первом включении устройства контроллер проверяет свои последнии три страницы флеш памяти и при отсутсвии данных в первой строке
// (первый символл соответсвующей строки-ячейки равен 0xFFFFFFFF) заполняет эти страницы дефолтными строками
void FLASH_Write_Default_String(void)
{
//    uint8_t i = 0; // перебор страниц флеш
//    uint8_t j = 0; // перебор строк ячеек внутри страницы
//    uint8_t k = 0; // счетчик записываемых 32-х битных слов
//    uint8_t m = 1; // счетчик записаных строк
//    uint32_t write_str[MAX_SIZE_STRING_32];
//    uint8_t string_of_num[4]; // строка содержащая номер входного сигнала (маловероятно, что число превысит 3-и порядка)
//    uint8_t string_prefix_size1 = strlen(std_string_prefix1);
//    uint8_t string_prefix_size2 = strlen(std_string_prefix2);

    if (0xFFFFFFFF != FLASH_Read(DATA_Pages[0])) // стертая флешь заполняется 0xFFFFFFFF
    {
        return;
    }

    // Для случая простой нумерации цифровых входов
    /*
    for (i=0; i<NUM_OF_STR_PAGE; i++) // перебераем все страницы флеш отведенные под хранение текстовых сообщений
    {
        for (j=0; j < NUM_OF_CELL_STR; j++) // перебираем все строки-ячейки в этих cтраницах
        {
            memset(write_str,'\0',MAX_SIZE_STRING_8);
            if(j%2)
            {
                memcpy(write_str, std_string_prefix1, string_prefix_size1);
                itoa(m , string_of_num, 10);
                m++; // условный номер входного сигнала
            }
            else
            {
                memcpy(write_str, std_string_prefix2, string_prefix_size2);
                itoa(m , string_of_num, 10);
            }
            strncat(write_str, string_of_num , 4);
            strncat(write_str, "\0", 1);
            for (k = 0; k < MAX_SIZE_STRING_32; k++)
            {
                FLASH_ProgramWord(DATA_Pages[i] + MAX_SIZE_STRING_8*j + 4*k , write_str[k]);
            }
        }
    }
    */

    //Под так называемую 5-ую систему
    FLASH_Write_Msg_String(0, 0, (uint8_t *)"Porog 2 CH4", 12);
    FLASH_Write_Msg_String(0, 1, (uint8_t *)"Porog 1 CH4", 12);
    FLASH_Write_Msg_String(1, 0, (uint8_t *)"Neispravnost CH4", 17);
    FLASH_Write_Msg_String(1, 1, (uint8_t *)"Neispravnost CH4", 17);

    FLASH_Write_Msg_String(2, 0, (uint8_t *)"Porog 2 CO", 11);
    FLASH_Write_Msg_String(2, 1, (uint8_t *)"Porog 1 CO", 11);
    FLASH_Write_Msg_String(3, 0, (uint8_t *)"Neispravnost CO", 16);
    FLASH_Write_Msg_String(3, 1, (uint8_t *)"Neispravnost CO", 16);

    FLASH_Write_Msg_String(4, 0, (uint8_t *)"Klapan zakrut", 14);
    FLASH_Write_Msg_String(4, 1, (uint8_t *)"Klapan zakrut", 14);

    FLASH_Write_Msg_String(5, 0, (uint8_t *)"Vzlom", 6);
    FLASH_Write_Msg_String(5, 1, (uint8_t *)"Vzlom", 6);

    FLASH_Write_Msg_String(6, 0, (uint8_t *)"Pozhar", 7);
    FLASH_Write_Msg_String(6, 1, (uint8_t *)"Pozhar", 7);

    // Сигналы аварий
    FLASH_Write_Msg_String(7, 0, (uint8_t *)"Vh Avar 1", 10);
    FLASH_Write_Msg_String(7, 1, (uint8_t *)"Vh Avar 1", 10);
    FLASH_Write_Msg_String(8, 0, (uint8_t *)"Vh Avar 2", 10);
    FLASH_Write_Msg_String(8, 1, (uint8_t *)"Vh Avar 2", 10);
    FLASH_Write_Msg_String(9, 0, (uint8_t *)"Vh Avar 3", 10);
    FLASH_Write_Msg_String(9, 1, (uint8_t *)"Vh Avar 3", 10);
    FLASH_Write_Msg_String(10, 0, (uint8_t *)"Vh Avar 4", 10);
    FLASH_Write_Msg_String(10, 1, (uint8_t *)"Vh Avar 4", 10);
    FLASH_Write_Msg_String(11, 0, (uint8_t *)"Vh Avar 5", 10);
    FLASH_Write_Msg_String(11, 1, (uint8_t *)"Vh Avar 5", 10);
    FLASH_Write_Msg_String(12, 0, (uint8_t *)"Vh Avar 6", 10);
    FLASH_Write_Msg_String(12, 1, (uint8_t *)"Vh Avar 6", 10);
    FLASH_Write_Msg_String(13, 0, (uint8_t *)"Vh Avar 7", 10);
    FLASH_Write_Msg_String(13, 1, (uint8_t *)"Vh Avar 7", 10);
    FLASH_Write_Msg_String(14, 0, (uint8_t *)"Vh Avar 8", 10);
    FLASH_Write_Msg_String(14, 1, (uint8_t *)"Vh Avar 8", 10);
    FLASH_Write_Msg_String(15, 0, (uint8_t *)"Vh Avar 9", 10);
    FLASH_Write_Msg_String(15, 1, (uint8_t *)"Vh Avar 9", 10);
    FLASH_Write_Msg_String(16, 0, (uint8_t *)"Vh Avar 10", 11);
    FLASH_Write_Msg_String(16, 1, (uint8_t *)"Vh Avar 10", 11);
    FLASH_Write_Msg_String(17, 0, (uint8_t *)"Vh Avar 11", 11);
    FLASH_Write_Msg_String(17, 1, (uint8_t *)"Vh Avar 11", 11);
    FLASH_Write_Msg_String(18, 0, (uint8_t *)"Vh Avar 12", 11);
    FLASH_Write_Msg_String(18, 1, (uint8_t *)"Vh Avar 12", 11);
    FLASH_Write_Msg_String(19, 0, (uint8_t *)"Vh Avar 13", 11);
    FLASH_Write_Msg_String(19, 1, (uint8_t *)"Vh Avar 13", 11);
    FLASH_Write_Msg_String(20, 0, (uint8_t *)"Vh Avar 14", 11);
    FLASH_Write_Msg_String(20, 1, (uint8_t *)"Vh Avar 14", 11);
    FLASH_Write_Msg_String(21, 0, (uint8_t *)"Vh Avar 15", 11);
    FLASH_Write_Msg_String(21, 1, (uint8_t *)"Vh Avar 15", 11);
    FLASH_Write_Msg_String(22, 0, (uint8_t *)"Vh Avar 16", 11);
    FLASH_Write_Msg_String(22, 1, (uint8_t *)"Vh Avar 16", 11);

    FLASH_Write_Msg_String(23, 0, (uint8_t *)"Neispravnost SIST", 18);
    FLASH_Write_Msg_String(23, 1, (uint8_t *)"Neispravnost SIST", 18);

    SysReset();
}
//***************************************************************************************************************************************

//***************************************************************************************************************************************
// функция проверки заполнена ли 5-ая по счету с конца страница флеш где хранится конфигурация и если она пуста, запись дефолтной конфигурации
// при первом включении устройства контроллер проверяет свою пятую с конца страницу флеш памяти и при отсутсвии данных в первом байте
// (первый символл соответсвующей строки-ячейки равен 0xFFFFFFFF) заполняет эту страницу дефолтными значениями байт конфигурации
void FLASH_Write_Default_Config(void)
{
    uint8_t i = 0; // перебор страниц флеш

    if (0xFFFFFFFF != FLASH_Read(start_DATA_Page_59)) // стертая флешь заполняется 0xFFFFFFFF
    {
        return;
    }

    for (i=0; i<NUM_OF_INPUT; i++) // перебераем столько ячеек страницы конфигурации цифровых входов во флеш сколько имеем входов
    {
        //FLASH_Write_Config_Byte(i, 0); // все остальное по нулям
        if ((i == 1)||(i == 3)||(i == 23))
        {
            FLASH_Write_Config_Byte(i, 2); // вход 2, 4 и 24 - активное состояние +питания
        }
        else
        {
            FLASH_Write_Config_Byte(i, 0); // все остальное по нулям
        }
    }
}
//***************************************************************************************************************************************

// КОСТЫЛЬ (из-за переполнения ОЗУ при записи во флеш)
void SysReset(void)
{
    volatile uint32_t i,j; // чуть чуть ждем
    for(i=0;i<0x100000;i++);
    {
        for(j=0;j<0x100000;j++);
    }
    NVIC_SystemReset(); // сбрасываем для применения изменений
    return;
}

struct Flash_routine_st Flash_routine_state;
//***************************************************************************************************************************************
// Функция вызывается из main - проверяет флаги сохранения данных во флеш. И вызывает соответсвующие функции сохранения.
void WriteDataInFlash(void)
{
    if (Flash_routine_state.need_write.phone)
    {
        sys_timer_stop = 1; // тормозим вызовы в системном таймере
        Flash_routine_state.need_write.phone = 0;
        FLASH_Write_Phone_Num(Flash_routine_state.abonent_num, Flash_routine_state.phone_num, Flash_routine_state.phone_len + 1);
        SysReset();
        sys_timer_stop = 0; // после записи запускаем вызовы в системном таймере
    }

    if (Flash_routine_state.need_write.alarm_text1)
    {
        sys_timer_stop = 1; // тормозим вызовы в системном таймере
        Flash_routine_state.need_write.alarm_text1 = 0;
        FLASH_Write_Msg_String(Flash_routine_state.msg_num, 0, Flash_routine_state.Text1, Flash_routine_state.text_len + 1);
        SysReset();
        sys_timer_stop = 0; // после записи запускаем вызовы в системном таймере
    }

    if (Flash_routine_state.need_write.alarm_text2)
    {
        sys_timer_stop = 1; // тормозим вызовы в системном таймере
        Flash_routine_state.need_write.alarm_text2 = 0;
        FLASH_Write_Msg_String(Flash_routine_state.msg_num, 1, Flash_routine_state.Text2, Flash_routine_state.text_len + 1);
        SysReset();
        sys_timer_stop = 0; // после записи запускаем вызовы в системном таймере
    }

    if (Flash_routine_state.need_write.alarm_state)
    {
        sys_timer_stop = 1; // тормозим вызовы в системном таймере
        Flash_routine_state.need_write.alarm_state = 0;
        save_config_74HC165(&reg74hc165_current_state_num1); // сохраняем конфигурацию во флеш
        SysReset();
        sys_timer_stop = 0; // после записи запускаем вызовы в системном таймере
    }
    return;
}
//***************************************************************************************************************************************

