
#include "SData.h"
#include "uart.h"
#include "time.h"
#include "pc_mcu.h"
#include <stdio.h>
#include <string.h>
#include "gnss.h"
#include "camera.h"
// ***********************************************************
__IO FunctionalState g_nGPSTxEnable = DISABLE;
__IO FunctionalState g_nPCModeEnable = DISABLE;
__IO FunctionalState g_nCameraTxEnable = DISABLE;
// ***********************************************************
// ***********************************************************
void LED_Flash(void)
{
    static uint8_t nLedState = 0;
    if (nLedState == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_5); // 灭
        nLedState = 1;
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5); // 灯亮
        nLedState = 0;
    }
}
// ***********************************************************
uint8_t grmc_check(u8 *input, uint16_t nLen)
{
    uint16_t i = 0;
    uint8_t xor_temp = 0;
    for (i = 0; i < nLen; ++i)
    {
        xor_temp = xor_temp ^ input[i];
    }

    return xor_temp;
}
// ***********************************************************
// GPS模拟数据输出
void GPS_Output(void)
{
    static char rmc_temp[100] = {0};
    uint8_t check = 0;
    uint8_t nLen = 0;
    stRTC_Time stSysTime;
    usrGetTime(&stSysTime);

    snprintf((char *)rmc_temp, 100, "GPRMC,%02d%02d%02d.00,A,3016.6982780,N,12002.3824423,E,2.025,119.5,%02d%02d%02d,0.0,E,D",
             stSysTime.hours, stSysTime.minutes, stSysTime.seconds, stSysTime.dates, stSysTime.months, stSysTime.years);

    check = grmc_check((uint8_t *)rmc_temp, strlen(rmc_temp));

    snprintf((char *)rmc_temp, 100, "$GPRMC,%02d%02d%02d.00,A,3016.6982780,N,12002.3824423,E,2.025,119.5,%02d%02d%02d,0.0,E,D*%02X\r\n",
             stSysTime.hours, stSysTime.minutes, stSysTime.seconds, stSysTime.dates, stSysTime.months, stSysTime.years, check);

    nLen = strlen(rmc_temp);

    // 串口输出数据
    DMAUart_Send(&g_Uart2TxBuf, (u8 *)rmc_temp, nLen);
    Uartx_Send(&g_Uart2TxBuf, (u8 *)rmc_temp, nLen);
}
// ***********************************************************
// 1ms运行一次
void Timer3_IRQ(void)
{
    stRTC_Time stSysTime;
    usrTimeUpdate();

    if (g_nSysWork == 1) // 开始采集
    {
        usrGetTime(&stSysTime);
        if (stSysTime.mseconds == 0)
        {
            GPIO_SetBits(GPIOC, GPIO_Pin_6);
            GPIO_SetBits(GPIOC, GPIO_Pin_7);
        }
        else if (stSysTime.mseconds == 1)
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_6);
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        }
        else if (stSysTime.mseconds == 5) // 开启输出模拟GPS数据
        {
            g_nGPSTxEnable = ENABLE;
        }
        else if (stSysTime.mseconds == 10)
        {
            LED_Flash();
        }

        // 相机同步设置
        CameraSynic(&stSysTime);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_5); // 灯亮
        CameraSynic_clear();
    }

    TIM_GNSS_CallBack();
}

// ***********************************************************
