
#include <stm32f10x.h>
#include "cpu.h"
#include "SData.h"
#include "uart.h"
#include "pc_mcu.h"
#include "imu.h"
#include "gnss.h"
#include "camera.h"
//***********************************************

int main(void)
{
    u32 i = 0;
    for (i = 0; i < 5000000; ++i)
    {
    }

    RCC_ClockConfig();

    Sys_Config();

    IWDG_Config(); // 开启看门狗

    while (1)
    {
        if (g_Uart1RxBuf.bFinish == 1) // 接收PC数据
        {
            proc_PCMsg();
        }

        if (g_Uart4RxBuf.bFinish == 1)
        {
            proc_IMU();
        }

        if (g_Uart2RxBuf.bFinish == 1)
        {
            proc_GNSS();
        }

        if (g_nCameraTxEnable)
        {
            Camera_Output();
            g_nCameraTxEnable = DISABLE;
        }

        if (g_nPCModeEnable)
        {
            PC_SendMode();
            g_nPCModeEnable = DISABLE;
        }

        if (g_nGPSTxEnable)
        {
            GPS_Output();
            g_nGPSTxEnable = DISABLE;
        }

        __WFI(); // 进入睡眠模式
    }
}
