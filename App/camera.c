// ***********************************************************
// ***********************************************************
#include <stm32f10x.h>
#include "imu.h"
#include "uart.h"
#include "SData.h"
#include "pc_mcu.h"
#include "camera.h"
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
static stRTC_Time s_tCameraTime = {0};
static uint16_t s_Frame_count = 0;
static uint16_t s_Synic_frame_counter = 100;
static uint16_t s_Pusle_time_counter = 0;
// ***********************************************************
// ***********************************************************
// ***********************************************************
// Camera数据输出
void Camera_Output(void)
{
    uint8_t nIndex = 0;
    uint8_t nXOR = 0;
    u16 nID = 0x2001;
    static uint8_t pFrame[80] = {0};

    pFrame[nIndex++] = 0xFA;
    pFrame[nIndex++] = 0x4D;
    pFrame[nIndex++] = 0x43;
    pFrame[nIndex++] = 0x55;
    pFrame[nIndex++] = 14; // 长度
    pFrame[nIndex++] = (nID >> 8) & 0xFF;
    pFrame[nIndex++] = nID & 0xFF;
    pFrame[nIndex++] = s_tCameraTime.years;
    pFrame[nIndex++] = s_tCameraTime.months;
    pFrame[nIndex++] = s_tCameraTime.dates;
    pFrame[nIndex++] = s_tCameraTime.hours;
    pFrame[nIndex++] = s_tCameraTime.minutes;
    pFrame[nIndex++] = s_tCameraTime.seconds;
    pFrame[nIndex++] = (s_tCameraTime.mseconds & 0xFF);
    pFrame[nIndex++] = (s_tCameraTime.mseconds >> 8) & 0xFF;
    pFrame[nIndex++] = (s_Frame_count & 0xFF);
    pFrame[nIndex++] = (s_Frame_count >> 8) & 0xFF;
    pFrame[nIndex++] = 0; // 电压
    pFrame[nIndex++] = 0;

    nXOR = Cal_XOR(pFrame + 4, 15);
    pFrame[nIndex++] = nXOR;
    pFrame[nIndex++] = 0x0A;
    pFrame[nIndex++] = 0x0D;

    DMAUart_Send(&g_Uart1TxBuf, pFrame, nIndex);
}
//*****************************************************
int CameraManuMode(stRTC_Time *ptSysTime)
{
    s_Pusle_time_counter++;
    if (s_Pusle_time_counter == 1)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);

        s_tCameraTime = *ptSysTime;
        g_nCameraTxEnable = ENABLE;
        return 0;
    }

    else if (s_Pusle_time_counter == 2)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
        return 0;
    }
    else if (s_Pusle_time_counter == g_PulseGap)
    {
        s_Pusle_time_counter = 0; // 一个触发周期结束
        return 1;
    }

    return 0;
}
//*****************************************************
void CameraAtuoMode(stRTC_Time *ptSysTime)
{
    if (g_CameraPulse == 1)
    {
        g_CameraPulse = 0;
        s_Synic_frame_counter = 1;
        s_Pusle_time_counter = 0;
    }

    s_Pusle_time_counter++;
    if (s_Pusle_time_counter == 1)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);

        s_tCameraTime = *ptSysTime;
        g_nCameraTxEnable = ENABLE;
    }
    else if (s_Pusle_time_counter == 2)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
    else
    {
        switch (s_Synic_frame_counter)
        {
        case 1:
            if (s_Pusle_time_counter == 1000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 2:
            if (s_Pusle_time_counter == 2000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 3:
            if (s_Pusle_time_counter == 3000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 4:
            if (s_Pusle_time_counter == 4000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 5:
            if (s_Pusle_time_counter == 5000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 6:
            if (s_Pusle_time_counter == 6000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 7:
            if (s_Pusle_time_counter == 7000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 8:
            if (s_Pusle_time_counter == 8000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 9:
            if (s_Pusle_time_counter == 9000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        case 10:
            if (s_Pusle_time_counter == 10000)
            {
                s_Synic_frame_counter++;
                s_Pusle_time_counter = 0;
            }
            break;
        default:
            if (s_Pusle_time_counter == g_PulseGap)
            {
                if (s_Synic_frame_counter < 100)
                    s_Synic_frame_counter++;

                s_Pusle_time_counter = 0;
            }
            break;
        }
    }
}
// ***********************************************************
void CameraSynic(stRTC_Time *ptSysTime)
{
    switch (g_nCameraMode) // 0x01自动模式;0x02手动模式
    {
    case 0x01: // 自动
        CameraAtuoMode(ptSysTime);
        break;
    case 0x02:                 // 手动
        if (g_nCameraTri == 1) // 相机手动触发指令: 0无；1触发
        {
            if (CameraManuMode(ptSysTime)) // 触发完成后，就不触发了
            {
                g_nCameraTri = 0;
            }
        }
        break;
    default:
        break;
    }
}
// ***********************************************************
void CameraSynic_clear(void)
{
    g_CameraPulse = 0;
    s_Frame_count = 0;
    s_Synic_frame_counter = 100;
    s_Pusle_time_counter = 0;
}
// ***********************************************************
