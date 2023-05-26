
#include "uart.h"

// ***********************************************************
#define   NULL     0
// ***********************************************************
// ***********************************************************
// 串口1 TX:DMA1_CH4; RX:DMA1_CH5
// 串口2 TX:DMA1_CH6; RX:DMA1_CH7
// 串口3 TX:DMA1_CH2; RX:DMA1_CH3
// 串口4 TX:DMA2_CH3; RX:DMA2_CH5
// 串口5 不支持DMA
// ***********************************************************
// ***********************************************************
static u8   DMATxBuf1[UART1_DMA_TX_SIZE];
static u8   DMATxBuf2[UART2_DMA_TX_SIZE];
static u8   DMATxBuf3[UART3_DMA_TX_SIZE];
static u8   DMATxBuf4[UART4_DMA_TX_SIZE];
// ***********************************************************
static u8   DMARxBuf1[UART1_DMA_RX_SIZE];
static u8   DMARxBuf2[UART2_DMA_RX_SIZE];
static u8   DMARxBuf3[UART3_DMA_RX_SIZE];
static u8   DMARxBuf4[UART4_DMA_RX_SIZE];
// ***********************************************************
static u8   TxBuf1[UART1_TXBUF_SIZE];
static u8   TxBuf2[UART2_TXBUF_SIZE];
static u8   TxBuf3[UART3_TXBUF_SIZE];
static u8   TxBuf4[UART4_TXBUF_SIZE];
static u8   TxBuf5[UART5_TXBUF_SIZE];
// ***********************************************************
static u8   RxBuf1[UART1_RXBUF_SIZE];
static u8   RxBuf2[UART2_RXBUF_SIZE];
static u8   RxBuf3[UART3_RXBUF_SIZE];
static u8   RxBuf4[UART4_RXBUF_SIZE];
//static u8   RxBuf5[UART5_RXBUF_SIZE];
// ***********************************************************
UART_BUF    g_Uart1TxBuf = {UART1_TXBUF_SIZE, UART1_DMA_TX_SIZE, 0, 0, 0, 0, TxBuf1, DMATxBuf1, USART1, DMA1_Channel4, DMA1_IT_TC4};
UART_BUF    g_Uart2TxBuf = {UART2_TXBUF_SIZE, UART2_DMA_TX_SIZE, 0, 0, 0, 0, TxBuf2, DMATxBuf2, USART2, DMA1_Channel6, DMA1_IT_TC6};
UART_BUF    g_Uart3TxBuf = {UART3_TXBUF_SIZE, UART3_DMA_TX_SIZE, 0, 0, 0, 0, TxBuf3, DMATxBuf3, USART3, DMA1_Channel2, DMA1_IT_TC2};
UART_BUF    g_Uart4TxBuf = {UART4_TXBUF_SIZE, UART4_DMA_TX_SIZE, 0, 0, 0, 0, TxBuf4, DMATxBuf4, UART4,  DMA2_Channel3, DMA2_IT_TC3};
UART_BUF    g_Uart5TxBuf = {UART5_TXBUF_SIZE, 0,                 0, 0, 0, 0, TxBuf5, NULL,      UART5,  NULL,          DMA1_FLAG_TC5};
// ***********************************************************
// ***********************************************************
UART_BUF    g_Uart1RxBuf = {UART1_RXBUF_SIZE, UART1_DMA_RX_SIZE, 0, 0, 0, 0, RxBuf1, DMARxBuf1, USART1, DMA1_Channel5, DMA1_FLAG_GL5};
UART_BUF    g_Uart2RxBuf = {UART2_RXBUF_SIZE, UART2_DMA_RX_SIZE, 0, 0, 0, 0, RxBuf2, DMARxBuf2, USART2, DMA1_Channel7, DMA1_FLAG_GL7};
UART_BUF    g_Uart3RxBuf = {UART3_RXBUF_SIZE, UART3_DMA_RX_SIZE, 0, 0, 0, 0, RxBuf3, DMARxBuf3, USART3, DMA1_Channel3, DMA1_FLAG_GL3};
UART_BUF    g_Uart4RxBuf = {UART4_RXBUF_SIZE, UART4_DMA_RX_SIZE, 0, 0, 0, 0, RxBuf4, DMARxBuf4, UART4,  DMA2_Channel5, DMA2_FLAG_GL5};
//UART_BUF    g_Uart5RxBuf = {UART5_RXBUF_SIZE, 0,                 0, 0, 0, 0, RxBuf5, MILL,      UART5,  NULL,         NULL};
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// DMA中断
void DMAUart_TC_ISR(UART_BUF* pUartBuf)
{
	u16 i = 0;
    u16 nLen = 0;
    if(pUartBuf->pDMA_Streamx == NULL)
        return;
    
    nLen = pUartBuf->nCount;
    if(nLen > pUartBuf->nDMASize)
        nLen = pUartBuf->nDMASize;
    
    if(nLen > 0)
	{
		for(i = 0; i < nLen; ++i)
		{
			pUartBuf->pDMABuf[i] = pUartBuf->pBuf[pUartBuf->nRdInde++];
            if(pUartBuf->nRdInde >= pUartBuf->nSize)
                pUartBuf->nRdInde = 0;	
		}
        
        pUartBuf->nCount -= nLen;
		DMA_SetCurrDataCounter(pUartBuf->pDMA_Streamx, nLen);  // 数据传输量 
		DMA_Cmd(pUartBuf->pDMA_Streamx, ENABLE);               // 开启DMA传输 
	}
	else
	{
		pUartBuf->bFinish = 0;
	}
}
// ***********************************************************
// 数据发送
void DMAUart_Send(UART_BUF* pUartBuf, u8* pData, u16 nSize)
{
	u16 i = 0;
	if(nSize <= 0)
		return;
    
    if(pUartBuf->pDMA_Streamx == NULL)
        return;
    
    for(i = 0; i < nSize; ++i)
    {
        pUartBuf->pBuf[pUartBuf->nWrInde++] = pData[i];
        if(pUartBuf->nWrInde >= pUartBuf->nSize)
            pUartBuf->nWrInde = 0;
    }
	
    DMA_ITConfig(pUartBuf->pDMA_Streamx, DMA_IT_TC, DISABLE);
    if((nSize + pUartBuf->nCount) > pUartBuf->nSize)
        pUartBuf->nCount = pUartBuf->nSize;
	else
		pUartBuf->nCount += nSize; 
	DMA_ITConfig(pUartBuf->pDMA_Streamx, DMA_IT_TC, ENABLE);
    
    if(pUartBuf->bFinish == 1)
		return;
	
	pUartBuf->bFinish = 1;
	DMAUart_TC_ISR(pUartBuf);
}
// ***********************************************************
// ***********************************************************
// ***********************************************************
// 接收数据
void DMAUart_Rx_ISR(UART_BUF* pUartBuf)
{
    u8 nLen = 0, i = 0;
	nLen = pUartBuf->nDMASize - DMA_GetCurrDataCounter(pUartBuf->pDMA_Streamx);
	if(nLen > 0) // 拷贝数据
	{
		for(i = 0; i < nLen; ++i)
		{
			pUartBuf->pBuf[pUartBuf->nWrInde++] = pUartBuf->pDMABuf[i];
			if(pUartBuf->nWrInde >= pUartBuf->nSize)
				pUartBuf->nWrInde = 0;
		}
	
		pUartBuf->nCount += nLen;
		if(pUartBuf->nCount > pUartBuf->nSize)
		{
			pUartBuf->nCount = pUartBuf->nSize;
		}
	
		pUartBuf->bFinish = 1;
	}
	
	// 每次从DMA2读取完数据，都要重新配置一次DMA2的接收长度
	DMA_SetCurrDataCounter(pUartBuf->pDMA_Streamx, pUartBuf->nDMASize);
}
// ***********************************************************
u16 DMAUart_Read(UART_BUF* pUartBuf, u8* pData, u16 nSize)
{
    u16 i = 0;
    u16 nLen = 0;
    
	nLen = pUartBuf->nCount;
    if(nLen <= 0) // 没有数据
        return 0;
    
     if(nLen > nSize)
        nLen = nSize;
    
    for(i = 0; i < nLen; ++i)
    {
        pData[i] = pUartBuf->pBuf[pUartBuf->nRdInde++];
        if(pUartBuf->nRdInde >= pUartBuf->nSize)
            pUartBuf->nRdInde = 0;
    }
    
    USART_ITConfig(pUartBuf->pUSARTx, USART_IT_IDLE, DISABLE);
    pUartBuf->nCount -= nLen;
    if(pUartBuf->nCount <= 0)
        pUartBuf->bFinish = 0;
    USART_ITConfig(pUartBuf->pUSARTx, USART_IT_IDLE, ENABLE);
    
    return nLen;
}
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// 设置485设置
static void RS485_SetBit(USART_TypeDef* pUSARTx, u8 nSet)
{
    if(pUSARTx == USART3)
    {
        if(nSet != 0)
            GPIO_SetBits(GPIOE, GPIO_Pin_9);    // 发送
        else
            GPIO_ResetBits(GPIOE, GPIO_Pin_9);  // 接收
    }
}
// ***********************************************************
// 串口中断
void Uartx_TC_ISR(UART_BUF* pUartBuf)
{
    if(pUartBuf->pUSARTx == NULL)
        return;
    
    if(pUartBuf->nCount > 0)
	{
        USART_SendData(pUartBuf->pUSARTx, pUartBuf->pBuf[pUartBuf->nRdInde++]);
        if(pUartBuf->nRdInde >= pUartBuf->nSize)
            pUartBuf->nRdInde = 0;
        
        pUartBuf->nCount--;
	}
	else
	{
		USART_ITConfig(pUartBuf->pUSARTx, USART_IT_TC, DISABLE); 
        USART_ClearFlag(pUartBuf->pUSARTx, USART_FLAG_TC);
        RS485_SetBit(pUartBuf->pUSARTx, 0); // 485设置
        pUartBuf->bFinish = 0; 
	}
}
// ***********************************************************
// 数据发送
void Uartx_Send(UART_BUF* pUartBuf, u8* pData, u16 nSize)
{
	u16 i = 0;
    u16 nCount = 0;
	if(nSize <= 0)
		return;
    
    if(pUartBuf->pUSARTx == NULL)
        return;
    
    for(i = 0; i < nSize; ++i)
    {
        pUartBuf->pBuf[pUartBuf->nWrInde++] = pData[i];
        if(pUartBuf->nWrInde >= pUartBuf->nSize)
            pUartBuf->nWrInde = 0;
    }
	
	RS485_SetBit(pUartBuf->pUSARTx, 1); // 485设置,232不需要
    USART_ITConfig(pUartBuf->pUSARTx, USART_IT_TC, DISABLE);
    nCount = pUartBuf->nCount;
    if((pUartBuf->nCount + nSize) > pUartBuf->nSize)
        pUartBuf->nCount = pUartBuf->nSize;
	else
		pUartBuf->nCount += nSize;
    
    if(nCount <= 0)
    {
        USART_SendData(pUartBuf->pUSARTx, pUartBuf->pBuf[pUartBuf->nRdInde++]);
        if(pUartBuf->nRdInde >= pUartBuf->nSize)
            pUartBuf->nRdInde = 0;
        
        pUartBuf->nCount--;
    }
    
    pUartBuf->bFinish = 1;
	USART_ITConfig(pUartBuf->pUSARTx, USART_IT_TC, ENABLE);
}
// ***********************************************************
// 接收数据
void Uartx_Rx_ISR(UART_BUF* pUartBuf, u8 ch)
{
    pUartBuf->pBuf[pUartBuf->nWrInde] = ch;
    if(pUartBuf->nWrInde >= pUartBuf->nSize)
        pUartBuf->nWrInde = 0;
    else
        pUartBuf->nWrInde++;
    
    if(pUartBuf->nCount < pUartBuf->nSize)
        pUartBuf->nCount++;
    
}
// ***********************************************************
u16 Uartx_Rx_Count(UART_BUF* pUartBuf)
{
    return pUartBuf->nCount;
}
// ***********************************************************
void Uartx_Rx_Clear(UART_BUF* pUartBuf)
{
    pUartBuf->nCount = 0;
    pUartBuf->nWrInde = 0;
    pUartBuf->nRdInde = 0;
    pUartBuf->bFinish = 0;
}
// ***********************************************************
u16 Uartx_Read(UART_BUF* pUartBuf, u8* pData, u16 nSize)
{
    u16 i = 0;
    u16 nLen = 0;
    
	nLen = pUartBuf->nCount;
    if(nLen <= 0) // 没有数据
        return 0;
    
     if(nLen > nSize)
        nLen = nSize;
    
    for(i = 0; i < nLen; ++i)
    {
        pData[i] = pUartBuf->pBuf[pUartBuf->nRdInde++];
        if(pUartBuf->nRdInde >= pUartBuf->nSize)
            pUartBuf->nRdInde = 0;
    }
    
    USART_ITConfig(pUartBuf->pUSARTx, USART_IT_RXNE, DISABLE);
    pUartBuf->nCount -= nLen;
    if(pUartBuf->nCount <= 0)
        pUartBuf->bFinish = 0;
    USART_ITConfig(pUartBuf->pUSARTx, USART_IT_RXNE, ENABLE);
    
    return nLen;
}












