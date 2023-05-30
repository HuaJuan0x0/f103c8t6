
#include "cpu.h"
#include "pc_mcu.h"
#include "uart.h"
#include "time.h"
#include "SData.h"
// *****************************************************************
// *****************************************************************
// *****************************************************************
// *****************************************************************
__IO u8 g_nSysWork = 0;       // 0：停止工作；1:校时成功，开始工作
__IO u8 g_nGPSOK = 0;         // GPS校时
__IO u8 g_nIMUState = 0;      // IMU标志
__IO u8 g_nGNSSState = 0;     // GNSS标志
__IO u8 g_nCameraMode = 0x01; // 0x01自动模式;0x02手动模式
__IO u8 g_nCameraTri = 0x00;  // 相机手动触发指令: 0无；1触发
__IO u8 g_CameraPulse = 0;
__IO u16 g_PulseGap = 1000; // 1s
// *****************************************************************
// *****************************************************************
// *****************************************************************
// *****************************************************************
// *****************************************************************
static void Parse_Data(uint8_t *pMem, uint8_t nLen)
{
    stRTC_Time sTime;
    uint16_t nID = 0;
    uint16_t nData = 0;

    nID = ((uint16_t)pMem[1] << 8) + pMem[2];

    switch (nID)
    {
    case 0x3001: // 开始测量指令
        if (g_nGPSOK == 0)
        {
            sTime.years = pMem[3];
            sTime.months = pMem[4];
            sTime.dates = pMem[5];
            sTime.hours = pMem[6];
            sTime.minutes = pMem[7];
            sTime.seconds = pMem[8];

            __disable_irq();
            usrSetTime(&sTime);
            usrTimeSetMSZero();
            __enable_irq();
        }
        g_nSysWork = 1;
        break;
    case 0x4001: // 停止指令
        g_nSysWork = 0;
        g_nCameraTri = 0x0;
        break;
    case 0x5001: // 相机模式：1自动模式，2手动模式
        if (g_nSysWork == 0)
        {
            if (pMem[3] == 0x01)
                g_nCameraMode = 0x01; // 自动
            else if (pMem[3] == 0x02)
                g_nCameraMode = 0x02; // 手动
        }
        break;
    case 0x5002: // 相机拍照指令
        if (g_nSysWork == 1)
        {
            if (g_nCameraMode == 0x02) // 手动模式下
                g_nCameraTri = 0x01;   // 触发指令
        }
        break;
    case 0x6001: // 返回模式指令
        g_nPCModeEnable = ENABLE;
        break;
    case 0x8001: // 变间隔模式
        g_CameraPulse = 1;
        break;
    case 0x5003: // 触发间隔 最小0.1s
        nData = ((unsigned short)pMem[4] << 8) + pMem[3];

        if (nData < 100)
            g_PulseGap = 100;
        else
            g_PulseGap = nData;
        break;
    default:
        break;
    }
}
// *****************************************************************
// 系统控制命令处理
static void PC_Msg(uint8_t nCh)
{
    const uint8_t UART_DATA_MAX_SIZE = 70;
    static uint8_t nFlag = 0;
    static uint8_t nIndex = 0;
    static uint8_t pMem[UART_DATA_MAX_SIZE] = {0};
    static uint8_t nLen = 0;
    uint8_t nXOR = 0;

    switch (nFlag)
    {
    case 0:
        if (nCh == 0xFA)
            nFlag = 1;
        break;
    case 1:
        if (nCh == 0x4D)
            nFlag = 2;
        else if (nCh == 0xFA)
            nFlag = 1;
        else
            nFlag = 0;
        break;
    case 2:
        if (nCh == 0x43)
            nFlag = 3;
        else if (nCh == 0xFA)
            nFlag = 1;
        else
            nFlag = 0;
        break;
    case 3:
        if (nCh == 0x55)
            nFlag = 4;
        else if (nCh == 0xFA)
            nFlag = 1;
        else
            nFlag = 0;
        break;
    case 4: // 长度
        nLen = nCh;
        nIndex = 0;
        pMem[nIndex++] = nCh;
        nFlag = 5;
        break;
    case 5:
        pMem[nIndex++] = nCh;
        if (nIndex >= (nLen + 1))
        {
            nFlag = 6;
        }

        if (nIndex >= (UART_DATA_MAX_SIZE - 5))
            nFlag = 0;
        break;
    case 6:
        nXOR = Cal_XOR(pMem, nIndex);
        if (nXOR == nCh) // 数据解析
        {
            Parse_Data(pMem, nIndex);
        }
        nFlag = 0;
        break;
    default:
        break;
    }
}
// ***********************************************************
void proc_PCMsg(void)
{
    uint16_t i = 0;
    const uint16_t MAX_LEN = 60;
    static uint8_t pBuf[MAX_LEN] = {0};
    uint16_t nLen = 0;

    nLen = DMAUart_Read(&g_Uart1RxBuf, pBuf, MAX_LEN);
    for (i = 0; i < nLen; ++i)
    {
        PC_Msg(pBuf[i]);
    }
}
// ***********************************************************
// 发送电脑模式
void PC_SendMode(void)
{
    uint8_t nIndex = 0;
    uint8_t nXOR = 0;
    const u16 nID = 0x6002;
    static uint8_t pFrame[40] = {0};

    pFrame[nIndex++] = 0xFA;
    pFrame[nIndex++] = 0x4D;
    pFrame[nIndex++] = 0x43;
    pFrame[nIndex++] = 0x55;
    pFrame[nIndex++] = 3; // 长度
    pFrame[nIndex++] = (nID >> 8) & 0xFF;
    pFrame[nIndex++] = nID & 0xFF;
    pFrame[nIndex++] = g_nCameraMode;
    nXOR = Cal_XOR(pFrame + 4, 4);
    pFrame[nIndex++] = nXOR;
    pFrame[nIndex++] = 0x0A;
    pFrame[nIndex++] = 0x0D;

    DMAUart_Send(&g_Uart1TxBuf, pFrame, nIndex);
}
// *********************************************************** end
