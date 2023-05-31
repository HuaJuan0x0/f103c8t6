
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "cpu.h"
#include "uart.h"
#include "time.h"
#include "gnss.h"
#include "imu.h"
#include "pc_mcu.h"

// ***********************************************************
// ***********************************************************
// ***********************************************************
static __IO u16 s_nTime_GNSS_Num = 0; // n ms传输一次触发时间
static __IO u8 s_nGNSS_Tick_second = 0;
static __IO u32 s_nGNSSSysTick = 0;
static __IO u32 s_nGNSSTick = 0;
static stRTC_Time s_stGNSSTime;
static __IO u16 s_Data_Tick = 0;
// ***********************************************************
// ***********************************************************
// ***********************************************************
// *****************************************************************
// 数据分割，可以分割两个连续的分隔符
static char *strsplit(char **stringp, const char *delim)
{
    char *start = *stringp;
    char *p;

    p = (start != NULL) ? strpbrk(start, delim) : NULL;

    if (p == NULL)
    {
        *stringp = NULL;
    }
    else
    {
        *p = '\0';
        *stringp = p + 1;
    }

    return start;
}
// GGA数据解析
static GGA gga_data_parse(char *gga_data)
{
    GGA gga;
    unsigned char times = 0;
    char *p;
    char *end;
    char *s = strdup(gga_data);

    p = strsplit(&s, ",");
    while (p)
    {
        switch (times)
        {
        case 1: // UTC
            strcpy(gga.utc, p);
            break;
        case 2: // lat
            gga.lat = strtod(p, NULL);
            break;
        case 3: // lat dir
            gga.lat_dir = p[0];
            break;
        case 4: // lon
            gga.lon = strtod(p, NULL);
            break;
        case 5: // lon dir
            gga.lon_dir = p[0];
            break;
        case 6: // quality
            gga.quality = (unsigned char)strtol(p, NULL, 10);
            break;
        case 7: // sats
            gga.sats = (unsigned char)strtol(p, NULL, 10);
            break;
        case 8: // hdop
            gga.hdop = (unsigned char)strtol(p, NULL, 10);
            break;
        case 9: // alt
            gga.alt = strtof(p, NULL);
            break;
        case 11: // undulation
            gga.undulation = strtof(p, NULL);
            break;
        case 13: // age
            gga.age = (unsigned char)strtol(p, NULL, 10);
            break;
        case 14: // stn_ID
            end = (char *)malloc(sizeof(p));
            strncpy(end, p, strlen(p) - 3);
            end[strlen(p) - 3] = '\0';
            gga.stn_ID = (unsigned short)strtol(end, NULL, 10);
            free(end);
            break;
        default:
            break;
        }
        p = strsplit(&s, ",");
        times++;
    }
    free(s);
    return gga;
}

static stRTC_Time rtc_parse(char *time)
{
    usrGetTime(&s_stGNSSTime);
    double time_float;
    time_float = strtod(time, NULL);
    s_stGNSSTime.hours = (unsigned int)time_float / 10000;
    s_stGNSSTime.minutes = (unsigned int)time_float % 10000 / 100;
    s_stGNSSTime.seconds = (unsigned int)time_float % 100;
    // s_stGNSSTime.mseconds = (unsigned short)(time_float - (unsigned int)time_float);

    return s_stGNSSTime;
}
// GGA和ZDA数据提取出来
// $GPGGA,HHMMSS.SS,DDMM.MMMM,S,DDDMM.MMMM,S,N,QQ,PP.P,SAAAAA.AA,M,±XXXX.XX,M,SSS,AAAA*CC<CR><LF>
// $GPZDA,HHMMSS.SS,DD,MM,YYYY,XX,YY*CC<CR><LF>
// 字符处理
void proc_gnssdata(char ch)
{
#define DATA_HEAD_SIZE 15

    static uint8_t nState = 0;
    static uint8_t pData[200] = {0};
    static uint8_t *pGGABuf = &pData[DATA_HEAD_SIZE];
    static uint8_t *pZDABuf = &pData[115];
    static uint8_t nGGAIndex = 0;
    static uint8_t nZDAIndex = 0;
    static uint8_t nType = 1;
    static uint8_t nGGAOK = 0;
    static uint8_t nZDAOK = 0;
    static u32 nGNSSDataTick = 0;
    uint16_t nIndex = 0;
    uint16_t i = 0;
    uint8_t nSum = 0;

    if (g_nSysWork == 0)
    {
        nState = 0;
        nGGAOK = 0;
        nZDAOK = 0;
        return;
    }

    switch (nState)
    {
    case 0: // $GPGGA，或$GPZDA
        if (ch == '$')
        {
            nState = 1;
        }
        break;
    case 1:
        if (ch == 'G')
            nState = 2;
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 2:
        if (ch == 'P')
        {
            nState = 3;
            nType = 1;
        }
        else if (ch == 'N')
        {
            nState = 3;
            nType = 2;
        }
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 3:
        if (ch == 'G')
            nState = 4;
        else if (ch == 'Z')
            nState = 100;
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 4:
        if (ch == 'G')
            nState = 5;
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 5:
        if (ch == 'A')
        {
            nGNSSDataTick = s_nGNSSSysTick;
            nGGAOK = 0;
            nState = 6;
            nGGAIndex = 0;
            pGGABuf[nGGAIndex++] = '$';
            pGGABuf[nGGAIndex++] = 'G';
            pGGABuf[nGGAIndex++] = (nType == 2 ? 'N' : 'P');
            pGGABuf[nGGAIndex++] = 'G';
            pGGABuf[nGGAIndex++] = 'G';
            pGGABuf[nGGAIndex++] = 'A';
        }
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 100:
        if (ch == 'D')
            nState = 101;
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 101:
        if (ch == 'A')
        {
            nZDAOK = 0;
            nState = 102;
            nZDAIndex = 0;
            pZDABuf[nZDAIndex++] = '$';
            pZDABuf[nZDAIndex++] = 'G';
            pZDABuf[nZDAIndex++] = (nType == 2 ? 'N' : 'P');
            pZDABuf[nZDAIndex++] = 'Z';
            pZDABuf[nZDAIndex++] = 'D';
            pZDABuf[nZDAIndex++] = 'A';
        }
        else if (ch == '$')
            nState = 1;
        else
            nState = 0;
        break;
    case 6: // 开始接收GGA数据
        pGGABuf[nGGAIndex++] = ch;
        if ((ch == 0x0A) && (pGGABuf[nGGAIndex - 2] == 0x0D)) // 结束接收的数据
        {
            nState = 0;
            nGGAOK = 1;
        }
        // Q&A:
        // Q:130？pGGABuf->pData[15:115)
        if (nGGAIndex > 130)
            nState = 0;

        break;
    case 102: // 开始接收ZDA数据
        pZDABuf[nZDAIndex++] = ch;
        if ((ch == 0x0A) && (pZDABuf[nZDAIndex - 2] == 0x0D)) // 结束接收的数据
        {
            nState = 0;
            nZDAOK = 1;
        }
        // Q&A:
        // Q:100?pZDABuf->pData[115:200)
        if (nZDAIndex > 100)
            nState = 0;

        break;
    default:
        break;
    }

    // 将串口拿到的gga数据解析
    if (nGGAOK)
    {
        char *gga_data = pGGABuf;
        GGA gga_stData = gga_data_parse(gga_data);
        s_stGNSSTime = rtc_parse(gga_stData.utc);

        __disable_irq();
        usrSetTime(&s_stGNSSTime);
        // usrTimeSetMSZero();
        __enable_irq();
    }

    // GNSS数据发送出去
    if (s_nTime_GNSS_Num < 900)
    {
        // #NOTE: nGNSSDataTick > s_nGNSSTick 舍弃1s传输超时 gngga
        if ((nGGAOK == 1) && (nZDAOK == 1) && (s_nGNSS_Tick_second == 1) && (nGNSSDataTick > s_nGNSSTick))
        {
            s_nGNSS_Tick_second = 0;
            nGGAOK = 0;
            nZDAOK = 0;

            nIndex = 0;
            pData[nIndex++] = 0xFA;
            pData[nIndex++] = 0x4D;
            pData[nIndex++] = 0x43;
            pData[nIndex++] = 0x55;
            pData[nIndex++] = 10 + nGGAIndex + nZDAIndex; // 长度
            pData[nIndex++] = 0x30;
            pData[nIndex++] = 0x01;
            pData[nIndex++] = s_stGNSSTime.years;
            pData[nIndex++] = s_stGNSSTime.months;
            pData[nIndex++] = s_stGNSSTime.dates;
            pData[nIndex++] = s_stGNSSTime.hours;
            pData[nIndex++] = s_stGNSSTime.minutes;
            pData[nIndex++] = s_stGNSSTime.seconds;
            pData[nIndex++] = s_stGNSSTime.mseconds >> 8;
            pData[nIndex++] = s_stGNSSTime.mseconds;

            nIndex += nGGAIndex;
            for (i = 0; i < nZDAIndex; i++)
                pData[nIndex++] = pZDABuf[i];

            nSum = Cal_SUM(pData + 4, nIndex - 4);
            pData[nIndex++] = nSum;
            pData[nIndex++] = 0x0A;
            pData[nIndex++] = 0x0D;

            DMAUart_Send(&g_Uart1TxBuf, pData, nIndex);
            g_nGNSSState |= 0x02;
        }
    }
}

// *****************************************************************
// PPS中断  PB1 GNSS PPS
void GNSS_PPS_Callback(void)
{
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == RESET) // 低电平
        return;
    usrGetTime(&s_stGNSSTime);
    usrTimeSetMSZero();
    // Q&A:
    // Q:550是什么？
    if (s_nTime_GNSS_Num <= 550)
        return;

    // usrGetTime(&s_stGNSSTime);
    s_nTime_GNSS_Num = 0;
    s_nGNSS_Tick_second = 1;
    s_nGNSSTick = s_nGNSSSysTick;
    s_Data_Tick = 0;
    g_nGNSSState |= 0x01;
}
// *****************************************************************
// GPS数据接收处理
void proc_GNSS(void)
{
    uint16_t i = 0;
    const uint16_t MAX_LEN = 100;
    static uint8_t pBuf[MAX_LEN] = {0};
    uint16_t nLen = 0;

    nLen = DMAUart_Read(&g_Uart2RxBuf, pBuf, MAX_LEN);
    for (i = 0; i < nLen; ++i)
    {
        proc_gnssdata(pBuf[i]);
    }

    // if (g_nSysWork == 1)
    // {
    //     for (i = 0; i < nLen; ++i)
    //     {
    //         proc_gnssdata(pBuf[i]);
    //     }
    // }
}
// *****************************************************************
void TIM_GNSS_CallBack(void)
{
    if (s_nTime_GNSS_Num < 65000)
        s_nTime_GNSS_Num++;

    s_nGNSSSysTick++;

    if (s_Data_Tick < 60000)
        s_Data_Tick++;

    if (s_Data_Tick >= 2000)
        g_nGNSSState = 0;
}
// *********************************************************** end
