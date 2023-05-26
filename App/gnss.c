
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
static __IO u16 s_nTime_GNSS_Num = 0;
static __IO u8  s_nGNSS_Tick_second = 0;
static __IO u32 s_nGNSSSysTick = 0;
static __IO u32 s_nGNSSTick = 0;
static  stRTC_Time  s_stGNSSTime;
static __IO u16 s_Data_Tick = 0;
// ***********************************************************
// ***********************************************************
// ***********************************************************
// *****************************************************************
// GGA和ZDA数据提取出来
// 字符处理
void proc_gnssdata(char ch)
{
#define   DATA_HEAD_SIZE    15
    
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
	
	if(g_nSysWork == 0)
    {
		nState = 0;
		nGGAOK = 0;
		nZDAOK = 0;
		return;
	}

    switch(nState)
    {
        case 0: // $GPGGA，或$GPZDA
            if(ch == '$')
            {
                nState = 1;
            }
            break;
        case 1:
            if(ch == 'G')
                nState = 2;
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 2:
            if(ch == 'P')
            {
				nState = 3;
				nType = 1;
			}
			else if(ch == 'N')
			{
				nState = 3;
				nType = 2;
			}
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 3:
            if(ch == 'G')
                nState = 4;
            else if(ch == 'Z')
                nState = 100;
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 4:
            if(ch == 'G')
                nState = 5;
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 5:
            if(ch == 'A')
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
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 100:
            if(ch == 'D')
                nState = 101;
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
        case 101:
            if(ch == 'A')
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
            else if(ch == '$')
                nState = 1;
            else
                 nState = 0;
            break;
         case 6:  // 开始接收GGA数据
            pGGABuf[nGGAIndex++] = ch;
            if((ch == 0x0A) && (pGGABuf[nGGAIndex - 2] == 0x0D)) // 结束接收的数据
            {
                nState = 0;
                nGGAOK = 1;
            }
            
             if(nGGAIndex > 130)
                nState = 0;
             
            break;
        case 102: //开始接收ZDA数据
            pZDABuf[nZDAIndex++] = ch;
            if((ch == 0x0A) && (pZDABuf[nZDAIndex - 2] == 0x0D)) // 结束接收的数据
            {
                nState = 0;
                nZDAOK = 1;
            }
            
            if(nZDAIndex > 100)
                nState = 0;
            
            break;
        default:
            break;
    }
    
    // GNSS数据发送出去
    if(s_nTime_GNSS_Num < 900)
    {        
        if((nGGAOK == 1) && (nZDAOK == 1) && (s_nGNSS_Tick_second == 1) && (nGNSSDataTick > s_nGNSSTick))
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
            pData[nIndex++] = 0x00;
            pData[nIndex++] = 0x03;    
            pData[nIndex++] = s_stGNSSTime.years;
            pData[nIndex++] = s_stGNSSTime.months;
            pData[nIndex++] = s_stGNSSTime.dates;
            pData[nIndex++] = s_stGNSSTime.hours;
            pData[nIndex++] = s_stGNSSTime.minutes;
            pData[nIndex++] = s_stGNSSTime.seconds;
            pData[nIndex++] = s_stGNSSTime.mseconds >> 8;
            pData[nIndex++] = s_stGNSSTime.mseconds;
    
            nIndex += nGGAIndex;
            for(i = 0; i < nZDAIndex; i++)
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
// PPS中断  PE1 GNSS PPS
void GNSS_PPS_Callback(void)
{
    if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_1) == RESET)  // 低电平
		return;
    
    if(s_nTime_GNSS_Num <= 550)
        return;
    
    usrGetTime(&s_stGNSSTime);
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
    if(g_nSysWork == 1)
    {
        for(i = 0; i < nLen; ++i)
        {
            proc_gnssdata(pBuf[i]);
        }
    }
}
// *****************************************************************
void TIM_GNSS_CallBack(void)
{
    if(s_nTime_GNSS_Num < 65000)
        s_nTime_GNSS_Num++;

	s_nGNSSSysTick++;
    
    if(s_Data_Tick < 60000)
        s_Data_Tick++;
    
    if(s_Data_Tick >= 2000)
        g_nGNSSState = 0;
}
// *********************************************************** end

















