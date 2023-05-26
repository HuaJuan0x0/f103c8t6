#ifndef __CPU_H
#define __CPU_H

#define   UART1_BAUDRATE    115200
#define   UART2_BAUDRATE    9600
#define   UART3_BAUDRATE    115200
#define   UART4_BAUDRATE    115200
#define   UART5_BAUDRATE    9600

void RCC_ClockConfig(void);

void Sys_Config(void); // œµÕ≥≈‰÷√

void IWDG_Config(void);

#endif //__CPU_H

