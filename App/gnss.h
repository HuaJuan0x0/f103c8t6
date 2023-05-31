#ifndef __GNSS_H
#define __GNSS_H

void proc_GNSS(void); // PTP���ݽ��մ���
void GNSS_PPS_Callback(void);
void TIM_GNSS_CallBack(void);

#define PRE_GGA "$GPGGA"
#define PRE_ZDA "$GPZDA"
#define ENABLE_GGA 1
#define ENABLE_ZDA 0

#if ENABLE_GGA
// GGA�ֶνṹ�壨GPS��λ���ݣ�
typedef struct
{
    char utc[11];          // UTCʱ�䣬��ʽΪhhmmss.sss
    double lat;            // γ�ȣ���ʽΪddmm.mmmm
    char lat_dir;          // γ�Ȱ���N��S
    double lon;            // ���ȣ���ʽΪdddmm.mmmm
    char lon_dir;          // ���Ȱ���E��W
    unsigned char quality; // 0=��λ��Ч��1=��λ��Ч
    unsigned char sats;    // ʹ��������������00��12
    double hdop;           // ˮƽ��ȷ�ȣ�0.5��99.9����λm
    double alt;            // ��ƽ��ĸ߶ȣ�-9999.9��9999.9��
    double undulation;     // ���ˮ׼��߶ȣ�-9999.9��9999.9��
    unsigned char age;     // ���ʱ��
    unsigned short stn_ID; // ���վID��0000 - 1023
} GGA;
#endif

#endif //__GNSS_H
