#ifndef __GNSS_H
#define __GNSS_H

void proc_GNSS(void); // PTP数据接收处理
void GNSS_PPS_Callback(void);
void TIM_GNSS_CallBack(void);

#define PRE_GGA "$GPGGA"
#define PRE_ZDA "$GPZDA"
#define ENABLE_GGA 1
#define ENABLE_ZDA 0

#if ENABLE_GGA
// GGA字段结构体（GPS定位数据）
typedef struct
{
    char utc[11];          // UTC时间，格式为hhmmss.sss
    double lat;            // 纬度，格式为ddmm.mmmm
    char lat_dir;          // 纬度半球，N或S
    double lon;            // 经度，格式为dddmm.mmmm
    char lon_dir;          // 经度半球，E或W
    unsigned char quality; // 0=定位无效，1=定位有效
    unsigned char sats;    // 使用卫星数量，从00到12
    double hdop;           // 水平精确度，0.5到99.9，单位m
    double alt;            // 海平面的高度，-9999.9到9999.9米
    double undulation;     // 大地水准面高度，-9999.9到9999.9米
    unsigned char age;     // 差分时间
    unsigned short stn_ID; // 差分站ID号0000 - 1023
} GGA;
#endif

#endif //__GNSS_H
