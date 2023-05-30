#ifndef __TIME_H
#define __TIME_H

#include <stm32f10x.h>

typedef struct
{
	unsigned char years;
	unsigned char months;
	unsigned char dates;
	unsigned char hours;
	unsigned char minutes;
	unsigned char seconds;
	unsigned short int mseconds;
} stRTC_Time;

void usrTimeUpdate(void);
void usrTimeUpdateMS(void);
void usrTimeSetMSZero(void);
void usrGetTime(stRTC_Time *time); // 读取时间
void usrSetTime(stRTC_Time *time); // 设置时间
uint8_t Cal_SUM(uint8_t *pBuf, uint16_t nSize);
uint8_t Cal_XOR(uint8_t *pBuf, uint16_t nSize);

extern stRTC_Time g_ImuTime;
#endif //__TIME_H
