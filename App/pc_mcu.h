#ifndef __PC_MCU_H
#define __PC_MCU_H

#include <stm32f10x.h>

void proc_PCMsg(void);
void PC_SendMode(void);
void GPS_Output(void);

extern __IO u8  g_nSysWork;   // 0��ֹͣ������1:Уʱ�ɹ�����ʼ����
extern __IO u8  g_nGPSState;  // GPSУʱ��־
extern __IO u8  g_nGPSOK;     // GPSУʱ
extern __IO u8  g_nIMUState;  // IMU��־
extern __IO u8  g_nGNSSState; // GNSS��־
extern __IO u8  g_nCameraMode;// 0x01�Զ�ģʽ;0x02�ֶ�ģʽ
extern __IO u8  g_nCameraTri; // ����ֶ�����ָ��: 0�ޣ�1����
extern __IO u8  g_CameraPulse;
extern __IO u16 g_PulseGap;   // 1s

#endif //__PC_MCU_H

