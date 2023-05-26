#ifndef __SDATA_H
#define __SDATA_H

#include <stm32f10x.h>

typedef union
{
	int16_t   n16;
	uint16_t  u16;
	struct
	{
		uint8_t uL8;
		uint8_t uH8;
	}n8;
}UN_N16;


void Timer3_IRQ(void);
void GPS_Output(void); // GPS模拟数据输出

extern __IO FunctionalState g_nGPSTxEnable;
extern __IO FunctionalState g_nPCModeEnable;
extern __IO FunctionalState g_nCameraTxEnable;
#endif //__SDATA_H

