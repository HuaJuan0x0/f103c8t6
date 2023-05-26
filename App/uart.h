#ifndef __UART_H
#define __UART_H

#include <stm32f10x.h>


#define   UART1_RXBUF_SIZE     250
#define   UART2_RXBUF_SIZE     250
#define   UART3_RXBUF_SIZE     250
#define   UART4_RXBUF_SIZE     250
#define   UART5_RXBUF_SIZE     250

#define   UART1_TXBUF_SIZE     250
#define   UART2_TXBUF_SIZE     250
#define   UART3_TXBUF_SIZE     250
#define   UART4_TXBUF_SIZE     250
#define   UART5_TXBUF_SIZE     250


#define   UART1_DMA_TX_SIZE    150
#define   UART2_DMA_TX_SIZE    100
#define   UART3_DMA_TX_SIZE    100
#define   UART4_DMA_TX_SIZE    100
#define   UART5_DMA_TX_SIZE    100

#define   UART1_DMA_RX_SIZE    128
#define   UART2_DMA_RX_SIZE    128
#define   UART3_DMA_RX_SIZE    128
#define   UART4_DMA_RX_SIZE    128
#define   UART5_DMA_RX_SIZE    128


#define   DMA_IT_0    (DMA_IT_TCIF0 | DMA_IT_TEIF0 | DMA_IT_DMEIF0 | DMA_IT_HTIF0 | DMA_FLAG_DMEIF0)
#define   DMA_IT_1    (DMA_IT_TCIF1 | DMA_IT_TEIF1 | DMA_IT_DMEIF1 | DMA_IT_HTIF1 | DMA_FLAG_DMEIF1)
#define   DMA_IT_2    (DMA_IT_TCIF2 | DMA_IT_TEIF2 | DMA_IT_DMEIF2 | DMA_IT_HTIF2 | DMA_FLAG_DMEIF2)
#define   DMA_IT_3    (DMA_IT_TCIF3 | DMA_IT_TEIF3 | DMA_IT_DMEIF3 | DMA_IT_HTIF3 | DMA_FLAG_DMEIF3)
#define   DMA_IT_4    (DMA_IT_TCIF4 | DMA_IT_TEIF4 | DMA_IT_DMEIF4 | DMA_IT_HTIF4 | DMA_FLAG_DMEIF4)
#define   DMA_IT_5    (DMA_IT_TCIF5 | DMA_IT_TEIF5 | DMA_IT_DMEIF5 | DMA_IT_HTIF5 | DMA_FLAG_DMEIF5)


#define   DMA_FLAG_0  (DMA_FLAG_TCIF0 | DMA_FLAG_TEIF0 | DMA_FLAG_DMEIF0 | DMA_FLAG_HTIF0 | DMA_FLAG_DMEIF0)
#define   DMA_FLAG_1  (DMA_FLAG_TCIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_HTIF1 | DMA_FLAG_DMEIF1)
#define   DMA_FLAG_2  (DMA_FLAG_TCIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_HTIF2 | DMA_FLAG_DMEIF2)
#define   DMA_FLAG_3  (DMA_FLAG_TCIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_DMEIF3)
#define   DMA_FLAG_4  (DMA_FLAG_TCIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_DMEIF4)
#define   DMA_FLAG_5  (DMA_FLAG_TCIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_HTIF5 | DMA_FLAG_DMEIF5)


//UART结构体定义
typedef struct								    
{	
    u16  nSize;	           // 缓冲区数组：缓冲区大小
    u16  nDMASize;         // DMA 数量
	u8   bFinish;          // 是否完成
    u8   nWrInde;		   // 缓冲区数组：写指针
	u8   nRdInde;	       // 缓冲区数组：读指针
	u8   nCount;		   // 缓冲区数组：数据计数
	u8   *pBuf;			   // 缓冲区数组：首地址指针变量
    u8   *pDMABuf;         // DMA地址
	USART_TypeDef* pUSARTx;// 缓冲区对应的USART
    DMA_Channel_TypeDef* pDMA_Streamx;
    uint32_t DMA_IT;
}UART_BUF;


void DMAUart_TC_ISR(UART_BUF* pUartBuf);
void DMAUart_Rx_ISR(UART_BUF* pUartBuf);
u16 DMAUart_Read(UART_BUF* pUartBuf, u8* pData, u16 nSize);
void DMAUart_Send(UART_BUF* pUartBuf, u8* pData, u16 nSize);


void Uartx_TC_ISR(UART_BUF* pUartBuf);
void Uartx_Send(UART_BUF* pUartBuf, u8* pData, u16 nSize);
void Uartx_Rx_ISR(UART_BUF* pUartBuf, u8 ch);
u16  Uartx_Rx_Count(UART_BUF* pUartBuf);
void Uartx_Rx_Clear(UART_BUF* pUartBuf);
u16  Uartx_Read(UART_BUF* pUartBuf, u8* pData, u16 nSize);

extern UART_BUF g_Uart1TxBuf;
extern UART_BUF g_Uart2TxBuf;
extern UART_BUF g_Uart3TxBuf;
extern UART_BUF g_Uart4TxBuf;
extern UART_BUF g_Uart5TxBuf;

extern UART_BUF g_Uart1RxBuf;
extern UART_BUF g_Uart2RxBuf;
extern UART_BUF g_Uart3RxBuf;
extern UART_BUF g_Uart4RxBuf;
//extern UART_BUF g_Uart5RxBuf;

#endif //__UART_H

