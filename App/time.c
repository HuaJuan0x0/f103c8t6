
#include "time.h"
// ***********************************************************
// ***********************************************************
stRTC_Time  g_SysTime = {20, 1, 1, 0, 0, 0, 0};
stRTC_Time  g_ImuTime = {20, 1, 1, 0, 0, 0, 0};
// ***********************************************************
// ***********************************************************
#define TIME_S     999
#define TIME_MH    60
#define DAY_HOURE  24
#define LEAP_FEB   29
#define IS_LEAP_YEAR(A)  (((A%4) == 0)? 1:0)
// ***********************************************************
// ***********************************************************
// ***********************************************************
uint8_t Cal_SUM(uint8_t* pBuf, uint16_t nSize)
{
    uint8_t nSUM = 0;
    uint16_t i;
    for(i = 0; i < nSize; ++i)
    {
        nSUM += pBuf[i];
    }

    return nSUM;
}
// ***********************************************************
uint8_t Cal_XOR(uint8_t* pBuf, uint16_t nSize)
{
    uint8_t nXOR = pBuf[0];
    uint16_t i;
    for(i = 1; i < nSize; ++i)
    {
        nXOR ^= pBuf[i];
    }

    return nXOR;
}
// ***********************************************************
unsigned char get_month_days(unsigned char year,unsigned char month)
{
	unsigned char month_array[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	static unsigned char month_days;
	// 判断闰年的二月
	if(IS_LEAP_YEAR(year) && (month == 2))
	{
		month_days = LEAP_FEB; 
	}
	else
	{
		 month_days = month_array[month-1];
	}
	
	return month_days;
}
// ***********************************************************
void usrTimeUpdate(void)
{	
	if(g_SysTime.mseconds >= TIME_S)
	{
		g_SysTime.mseconds = 0;
		g_SysTime.seconds++;
		if(g_SysTime.seconds >= TIME_MH)
		{
			g_SysTime.seconds = 0;
			g_SysTime.minutes++;

			if(g_SysTime.minutes >= TIME_MH)
			{
				g_SysTime.minutes = 0;				
				g_SysTime.hours++;
				
				if(g_SysTime.hours >= DAY_HOURE)
				{			
					g_SysTime.hours = 0;
					g_SysTime.dates++;
          
					if(g_SysTime.dates >= get_month_days(g_SysTime.years, g_SysTime.months))
					{			
						g_SysTime.dates = 0;
						
                        g_SysTime.months++;
						if(g_SysTime.months >= 12)
						{			
							g_SysTime.months = 0;
							g_SysTime.years++;
						}
					}
				}
			}
		}
	}
	else
	{
		g_SysTime.mseconds++;	
	}
}
// ***********************************************************
// 毫秒清0
void usrTimeSetMSZero(void)
{
    g_SysTime.mseconds = 0;
}
// ***********************************************************
// 更新毫秒
void usrTimeUpdateMS(void)
{
	if(g_SysTime.mseconds < 999)
	{
		g_SysTime.mseconds++;
	}
}
// ***********************************************************
// 读时间
void usrGetTime(stRTC_Time* time)
{
	*time = g_SysTime;
}
// ***********************************************************
// 设置时间
void usrSetTime(stRTC_Time* time)
{
	g_SysTime.years = time->years;
	g_SysTime .months = time->months;
	g_SysTime.dates = time->dates;
	g_SysTime.hours = time->hours;
	g_SysTime.minutes = time->minutes;
	g_SysTime.seconds = time->seconds;
	//g_SysTime.mseconds = time->mseconds;
}







