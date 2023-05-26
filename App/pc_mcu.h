#ifndef __PC_MCU_H
#define __PC_MCU_H

#include <stm32f10x.h>

void proc_PCMsg(void);
void PC_SendMode(void);
void GPS_Output(void);

extern __IO u8  g_nSysWork;   // 0：停止工作；1:校时成功，开始工作
extern __IO u8  g_nGPSState;  // GPS校时标志
extern __IO u8  g_nGPSOK;     // GPS校时
extern __IO u8  g_nIMUState;  // IMU标志
extern __IO u8  g_nGNSSState; // GNSS标志
extern __IO u8  g_nCameraMode;// 0x01自动模式;0x02手动模式
extern __IO u8  g_nCameraTri; // 相机手动触发指令: 0无；1触发
extern __IO u8  g_CameraPulse;
extern __IO u16 g_PulseGap;   // 1s

#endif //__PC_MCU_H

