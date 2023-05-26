/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "uart.h"
#include "SData.h"
#include "gnss.h"
/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 
// ***********************************************************
 //TIM3中断
void TIM2_IRQHandler(void)  
{
    if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
	{	
		Timer3_IRQ();
		IWDG_ReloadCounter();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  		 
	}		 
}
// ***********************************************************
// 串口1
void USART1_IRQHandler(void)
{
	u8 ucData;
	UART_BUF* pUartBuf = &g_Uart1RxBuf;
	if(USART_GetFlagStatus(USART1, USART_FLAG_ORE) != RESET)  // 溢出错误标志位
	{	
        ucData = USART_ReceiveData(USART1);
		ucData = ucData ;
        USART_ClearITPendingBit(USART1, USART_IT_ORE);
	}
    
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)    // 接收中断
	{
		ucData = USART_ReceiveData(USART1);
		
		DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE);
		DMA_ClearFlag(pUartBuf->DMA_IT);
		DMAUart_Rx_ISR(pUartBuf);
		DMA_Cmd(pUartBuf->pDMA_Streamx, ENABLE);
	}
}
// ***********************************************************
// 串口2
void USART2_IRQHandler(void)
{
    u8 ucData;
	UART_BUF* pUartBuf = &g_Uart2RxBuf;
	if(USART_GetFlagStatus(USART2, USART_FLAG_ORE) != RESET)  // 溢出错误标志位
	{	
        ucData = USART_ReceiveData(USART2);
		ucData = ucData ;
        USART_ClearITPendingBit(USART2, USART_IT_ORE);
	}
    
	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)    // 接收中断
	{
		ucData = USART_ReceiveData(USART2);
		
		DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE);
		DMA_ClearFlag(pUartBuf->DMA_IT);
		DMAUart_Rx_ISR(pUartBuf);
		DMA_Cmd(pUartBuf->pDMA_Streamx, ENABLE);
	}
}
// ***********************************************************
// 串口3
void USART3_IRQHandler(void)
{
	u8 ucData;
	UART_BUF* pUartBuf = &g_Uart3RxBuf;
	if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)  // 溢出错误标志位
	{	
        ucData = USART_ReceiveData(USART3);
		ucData = ucData ;
        USART_ClearITPendingBit(USART3, USART_IT_ORE);
	}
    
	if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)    // 接收中断
	{
		ucData = USART_ReceiveData(USART3);
		
		DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE);
		DMA_ClearFlag(pUartBuf->DMA_IT);
		DMAUart_Rx_ISR(pUartBuf);
		DMA_Cmd(pUartBuf->pDMA_Streamx, ENABLE);
	}
}
// ***********************************************************
// 串口4
void UART4_IRQHandler(void)
{
	u8 ucData;
	UART_BUF* pUartBuf = &g_Uart4RxBuf;
	if(USART_GetFlagStatus(UART4, USART_FLAG_ORE) != RESET)  // 溢出错误标志位
	{	
        ucData = USART_ReceiveData(UART4);
		ucData = ucData ;
        USART_ClearITPendingBit(UART4, USART_IT_ORE);
	}
    
	if(USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)    // 接收中断
	{
		ucData = USART_ReceiveData(UART4);
		
		DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE);
		DMA_ClearFlag(pUartBuf->DMA_IT);
		DMAUart_Rx_ISR(pUartBuf);
		DMA_Cmd(pUartBuf->pDMA_Streamx, ENABLE);
	}
}
// ***********************************************************
// 串口5
void UART5_IRQHandler(void)
{
	u8 ucData;
	UART_BUF* pUartBuf = &g_Uart5TxBuf;
	if(USART_GetFlagStatus(UART5 , USART_FLAG_ORE) != RESET)  // 溢出错误标志位
	{	
        ucData = USART_ReceiveData(UART5);
		ucData = ucData ;
        USART_ClearITPendingBit(UART5, USART_IT_ORE);
	}

    if(USART_GetITStatus(UART5, USART_IT_TC) != RESET)      // 发送完成中断
    {
        USART_ClearFlag(UART5, USART_FLAG_TC);
        USART_ClearITPendingBit(UART5, USART_IT_TC);
		
		Uartx_TC_ISR(pUartBuf);
    }
}
// ***********************************************************
// 串口1 TX
void DMA1_Channel4_IRQHandler(void)
{
	UART_BUF* pUartBuf = &g_Uart1TxBuf;
    
    DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE); //DMA1_IT_TC4
    DMA_ClearITPendingBit(pUartBuf->DMA_IT); 
	DMA_ClearFlag(DMA1_FLAG_TC4);
	
    DMAUart_TC_ISR(pUartBuf);
}
// ***********************************************************
// 串口2 TX
void DMA1_Channel6_IRQHandler(void)
{
	UART_BUF* pUartBuf = &g_Uart2TxBuf;
    
    DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE); //DMA1_IT_TC4
    DMA_ClearITPendingBit(pUartBuf->DMA_IT); 
	DMA_ClearFlag(DMA1_FLAG_TC6);
	
    DMAUart_TC_ISR(pUartBuf);
}
// ***********************************************************
// 串口3 TX
void DMA1_Channel2_IRQHandler(void)
{
	UART_BUF* pUartBuf = &g_Uart3TxBuf;
    
    DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE); //DMA1_IT_TC4
    DMA_ClearITPendingBit(pUartBuf->DMA_IT); 
	DMA_ClearFlag(DMA1_FLAG_TC2);
	
    DMAUart_TC_ISR(pUartBuf);
}
// ***********************************************************
// 串口4 TX
void DMA2_Channel3_IRQHandler(void)
{
	UART_BUF* pUartBuf = &g_Uart4TxBuf;
    
    DMA_Cmd(pUartBuf->pDMA_Streamx, DISABLE); 
    DMA_ClearITPendingBit(pUartBuf->DMA_IT); 
	DMA_ClearFlag(DMA2_FLAG_TC3);
	
    DMAUart_TC_ISR(pUartBuf);
}
// ***********************************************************
//外部中断1服务程序  pps interrupt PE1	EXTI1_IRQHandler
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
	{
        GNSS_PPS_Callback();
        
        EXTI->PR = 1 << 1;  //清除LINE1上的中断标志位
        EXTI_ClearITPendingBit(EXTI_Line1); 
    }
} 
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
