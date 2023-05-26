// ***********************************************************
// ***********************************************************
#include <stm32f10x.h>
#include "imu.h"
#include "uart.h"
#include "time.h"
#include "pc_mcu.h"
#include <stdio.h>
#include <string.h>
#define BYTE2UINT(a,b)  (uint16_t)((((uint16_t)a)<<8)+((uint16_t)b))
// ***********************************************************
#define  QUAT_ID      0x2010      
#define  ACCE_ID      0x4020
#define  GYRO_ID      0x8020
#define  IMU_CODE     0x1001
// ***********************************************************
typedef union
{
	unsigned int n32;
    float        f32;
	struct
	{
		unsigned char n1;
		unsigned char n2;
		unsigned char n3;
		unsigned char n4;
	}n8;	
}UN_32;


// ***********************************************************
//static stRTC_Time g_tImuTime = {0};
static uint8_t s_pFrame[100] = {0};
// ***********************************************************
float ByteToFloat(unsigned char* byteArray)
{
	char i;
	unsigned char temp_f[4];
	
	for(i = 0;i < 4;i ++)
	{
		*(temp_f+i) = byteArray[3-i];
	}
	return *((float*)temp_f);
}
// ***********************************************************
// 16505惯导
void IMU_adi16505(u8* pData, u8 nLen, stRTC_Time* imu_time)
{
    u8 nSum = 0, i;
	float fTemp = 0;
    UN_32 n32;
    u8 nIndex = 0;
    
    if(nLen < 38)
        return;
    
    nSum = Cal_SUM(pData, 37);
    if(nSum != pData[37])
        return;
    
    s_pFrame[nIndex++] = 0xFA;
	s_pFrame[nIndex++] = 0x4D;
	s_pFrame[nIndex++] = 0x43;
	s_pFrame[nIndex++] = 0x55;
    s_pFrame[nIndex++] = 50;
    
    s_pFrame[nIndex++] = IMU_CODE >> 8;
	s_pFrame[nIndex++] = IMU_CODE & 0xFF;
	
    s_pFrame[nIndex++] = imu_time->years;
	s_pFrame[nIndex++] = imu_time->months;
	s_pFrame[nIndex++] = imu_time->dates;
	
	s_pFrame[nIndex++] = imu_time->hours;
	s_pFrame[nIndex++] = imu_time->minutes;
	s_pFrame[nIndex++] = imu_time->seconds;
	s_pFrame[nIndex++] = (unsigned char)(imu_time->mseconds);
	s_pFrame[nIndex++] = (unsigned char)(imu_time->mseconds >> 8);
     
    for(i = 0; i < 16; i++)
	{
        s_pFrame[nIndex++] = 0;
	}
	
	// imu16505 标识符号
	s_pFrame[8] = 0x01;
	s_pFrame[9] = 0x01;
	s_pFrame[10] = 0x01;
	s_pFrame[11] = 0x01;
    
    n32.n8.n1 = pData[20];
	n32.n8.n2 = pData[21];
	n32.n8.n3 = pData[22];
	n32.n8.n4 = pData[23];
	fTemp = 1.0 * n32.n32 / 4194304.0; // acc_x
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
	
	n32.n8.n1 = pData[24];
	n32.n8.n2 = pData[25];
	n32.n8.n3 = pData[26];
	n32.n8.n4 = pData[27];
	fTemp = 1.0 * n32.n32 / 4194304.0; // acc_y
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
	
	n32.n8.n1 = pData[28];
	n32.n8.n2 = pData[29];
	n32.n8.n3 = pData[30];
	n32.n8.n4 = pData[31];
	fTemp = 1.0 * n32.n32 / 4194304.0; // acc_z
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
    
    n32.n8.n1 = pData[8];
	n32.n8.n2 = pData[9];
	n32.n8.n3 = pData[10];
	n32.n8.n4 = pData[11];
	fTemp = 1.0 * n32.n32 / 2097152.0 / 180.0 * 3.1416; // gyro_x 转换成弧度
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
	
	n32.n8.n1 = pData[12];
	n32.n8.n2 = pData[13];
	n32.n8.n3 = pData[14];
	n32.n8.n4 = pData[15];
	fTemp = 1.0 * n32.n32 / 2097152.0 / 180.0 * 3.1416; // gyro_y 转换成弧度
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
	
	n32.n8.n1 = pData[16];
	n32.n8.n2 = pData[17];
	n32.n8.n3 = pData[18];
	n32.n8.n4 = pData[19];
	fTemp = 1.0 * n32.n32 / 2097152.0 / 180.0 * 3.1416; // gyro_z 转换成弧度
	n32.f32 = fTemp;
	s_pFrame[nIndex++] = n32.n8.n4;
	s_pFrame[nIndex++] = n32.n8.n3;
	s_pFrame[nIndex++] = n32.n8.n2;
	s_pFrame[nIndex++] = n32.n8.n1;
    
    nSum = Cal_XOR(s_pFrame + 4, nIndex - 4);
    s_pFrame[nIndex++] = nSum;
    s_pFrame[nIndex++] = 0x0A;
    s_pFrame[nIndex++] = 0x0D;
    
    DMAUart_Send(&g_Uart1TxBuf, s_pFrame, nIndex);
    g_nIMUState |= 0x01;
}
// ***********************************************************
// xsens惯导
void IMU_xsens(u8* pData, u8 nLen, stRTC_Time* imu_time)
{
    u8  data_len = 0;
    u8 nIndex = 0;
	u8 nSum = 0, i = 0;
    
    data_len = *(pData + 3);
	if(data_len + 5 > nLen)
		return;

	nSum = Cal_SUM(pData + 1, data_len + 4);
	if(nSum != 0 )
		return;
	
    s_pFrame[nIndex++] = 0xFA;
	s_pFrame[nIndex++] = 0x4D;
	s_pFrame[nIndex++] = 0x43;
	s_pFrame[nIndex++] = 0x55;
    s_pFrame[nIndex++] = 50;
    
    s_pFrame[nIndex++] = IMU_CODE >> 8;
	s_pFrame[nIndex++] = IMU_CODE & 0xFF;
	
    s_pFrame[nIndex++] = imu_time->years;
	s_pFrame[nIndex++] = imu_time->months;
	s_pFrame[nIndex++] = imu_time->dates;
	
	s_pFrame[nIndex++] = imu_time->hours;
	s_pFrame[nIndex++] = imu_time->minutes;
	s_pFrame[nIndex++] = imu_time->seconds;
	s_pFrame[nIndex++] = (u8)(imu_time->mseconds);
	s_pFrame[nIndex++] = (u8)(imu_time->mseconds >> 8);
	for(i = 0; i < 16; i++)
	{
		s_pFrame[nIndex++] = *(pData + 7 + i);
	}
	
	for(i = 0; i < 12; i++)
	{
		s_pFrame[nIndex++] = *(pData + 26 + i);
	}
	
	for(i = 0; i < 12; i++)
	{
		s_pFrame[nIndex++] = *(pData + 41 + i);
	}
    
    nSum = Cal_XOR(s_pFrame + 4, nIndex - 4);
    s_pFrame[nIndex++] = nSum;
    s_pFrame[nIndex++] = 0x0A;
    s_pFrame[nIndex++] = 0x0D;
    
    DMAUart_Send(&g_Uart1TxBuf, s_pFrame, nIndex);
    g_nIMUState |= 0x02;
}
// ***********************************************************
// 时间标签什么时间加
void proc_IMU(void)
{
    // 分析数据正确与否，然后把时间标签加进去
    u8 nLen = 0;
    stRTC_Time  stImuTime;
  
    const u8 MAX_LEN = 150;
    static u8 pData[MAX_LEN] = {0};
    nLen = DMAUart_Read(&g_Uart4RxBuf, pData, MAX_LEN); // 获得数据发送
    if(nLen < 4)
        return;
    
    if(g_nSysWork != 1)
        return;
    
    //stImuTime = g_tImuTime;
    usrGetTime(&stImuTime);
    if((pData[0] == 0x55) && (pData[1] == 0x55) && (pData[2] == 0x55) && (pData[3] == 0x55)) 
    { // adi16505
        IMU_adi16505(pData, nLen, &stImuTime);
    }
    else if((pData[0] == 0xFA) && (pData[1] == 0xFF) && (pData[2] == 0x36))
    {  // xsens
        IMU_xsens(pData, nLen, &stImuTime);
    }
    else
    {
        g_nIMUState = 0;
    }
}
//*****************************************************






