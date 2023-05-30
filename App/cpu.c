
#include <stm32f10x.h>
#include "cpu.h"
#include "uart.h"
// ***********************************************************
// ***********************************************************
// ***********************************************************
// ***********************************************************
// 70MHz
void RCC_ClockConfig(void)
{
	u16 i = 0;
	ErrorStatus HSEStartUpStatus;
	RCC_DeInit();

	RCC_HSEConfig(RCC_HSE_ON);

	// RCC_HSICmd(ENABLE);

	do
	{
		HSEStartUpStatus = RCC_WaitForHSEStartUp();
		i++;

	} while ((HSEStartUpStatus == ERROR) && (i != HSEStartUp_TimeOut));

	if (HSEStartUpStatus == SUCCESS)
	{

		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); // ʹ��flashԤ��ȡ������

		// ��Flash���ڵȴ�״̬��2����Ը�Ƶʱ�ӵģ��������RCCûֱ�ӹ�ϵ�����������Թ�
		FLASH_SetLatency(FLASH_Latency_2);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);  // HCLK = SYSCLK ���ø�������ʱ��=ϵͳʱ��
		RCC_PCLK2Config(RCC_HCLK_Div1);	  // PCLK2 = HCLK ���õ�������2ʱ��=��������ʱ��
		RCC_PCLK1Config(RCC_HCLK_Div2);	  // PCLK1 = HCLK/2 ���õ�������1��ʱ��=����ʱ�ӵĶ���Ƶ
		RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADCCLK = PCLK2/6 ����ADC����ʱ��=��������2ʱ�ӵ�����Ƶ
		// Set PLL clock output to 72MHz using HSE (8MHz) as entry clock
		// �������������ȱʧ�ˣ���ȴ�ܹؼ�
		// �������໷���ⲿ10Mhz����7��Ƶ��70Mhz
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_7);
		RCC_PLLCmd(ENABLE); // Enable PLL ʹ�����໷

		// Wait till PLL is ready �ȴ����໷����ȶ�
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
			;

		// Select PLL as system clock source �����໷�������Ϊϵͳʱ��
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// Wait till PLL is used as system clock source �ȴ�У��ɹ�
		while (RCC_GetSYSCLKSource() != 0x08)
			;
	}
}
// ***********************************************************
// GPIO��ʼ��
// ����1��30	PA9/TX1 	��Ƕ��ʽ����ͨ��
//       31		PA10/RX1
// ����2��12	PA2/TX2		��ʹ��
// 		 13		PA3/RX2		����GNSS_GGA	MCU��ʱ
// ����3��21	PB10/TX3	���ڣ�����Ԥ����	RS232��ƽ
// 		 22		PB11/RX3

// 		 19		PB1			PPS����
//		 14		PA4			����IO	������������
//		 15		PA5			LED	IO��-������-��
static void GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	// USART_InitTypeDef USART_InitStruct;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	// RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART2EN, ENABLE);
	// RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART3EN, ENABLE);
	// ����1��30	PA9/TX1 	��Ƕ��ʽ����ͨ��
	//       31		PA10/RX1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// ����2��12	PA2/TX2		��ʹ��
	// 		 13		PA3/RX2		����GNSS_GGA	MCU��ʱ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// ����3��21	PB10/TX3	���ڣ�����Ԥ����	RS232��ƽ
	// 		 22		PB11/RX3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// PPS PB1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	// TODO: gpio_mode �����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//		 14		PA4			����IO	������������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//		 15		PA5			LED	IO��-������-��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
// ***********************************************************
// ����1���յ�DMA���ã�DMA1��ͨ��5
void DMA_USART1_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart1RxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// ����(�����赽�ڴ�)
	DMA_InitStructure.DMA_BufferSize = g_Uart1RxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);							// ����DMA1��5ͨ��

	DMA_Cmd(DMA1_Channel5, ENABLE);
}

// ***********************************************************
// ����1�����DMA���ã�DMA1��ͨ��4
void DMA_USART1_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart1TxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// ����(���ڴ浽����)
	DMA_InitStructure.DMA_BufferSize = g_Uart1TxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);							// ����DMA1��4ͨ��

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel4, DISABLE);
}
// ***********************************************************
// ����1���� PA9:TX; PA10:RX
void USART1_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	//  Tx(PA9)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Rx(PA10)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	// TODO: gpio ģʽ
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART1_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART1, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_USART1_RX_Config();

	DMA_USART1_TX_Config();
}
// ***********************************************************
// ����2���յ�DMA���ã�DMA1��ͨ��7
void DMA_USART2_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart2RxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// ����(�����赽�ڴ�)
	DMA_InitStructure.DMA_BufferSize = g_Uart2RxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);							// ����DMA1��4ͨ��

	DMA_Cmd(DMA1_Channel7, ENABLE);
}

// ***********************************************************
// ����2���յ�DMA���ã�DMA1��ͨ��6
void DMA_USART2_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart2TxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// ����(���ڴ浽����)
	DMA_InitStructure.DMA_BufferSize = g_Uart2TxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);							// ����DMA1��4ͨ��

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel6, DISABLE);
}
// ***********************************************************
// ����2���� PA2:TX; PA3:RX
void USART2_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// Configure USART2 Tx (PA.02) as alternate function push-pull
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Configure USART2 Rx (PA.03) as input floating
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART2_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_USART2_RX_Config();

	DMA_USART2_TX_Config();
}
// ***********************************************************
// ����3���յ�DMA���ã�DMA1��ͨ��3
void DMA_USART3_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart3RxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// ����(�����赽�ڴ�)
	DMA_InitStructure.DMA_BufferSize = g_Uart3RxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);							// ����DMA1��4ͨ��

	DMA_Cmd(DMA1_Channel3, ENABLE);
}

// ***********************************************************
// ����3���յ�DMA���ã�DMA1��ͨ��2
void DMA_USART3_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA����

	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);		// �������ݼĴ�����ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart3TxBuf.pDMABuf;	// �ڴ��ַ(Ҫ����ı�����ָ��)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// ����(���ڴ浽����)
	DMA_InitStructure.DMA_BufferSize = g_Uart3TxBuf.nDMASize;				// �������ݵĴ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// �����ַ����
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// �ڴ��ַ����
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݵ�λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// �ڴ����ݵ�λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMAģʽ��һ�δ��䣬ѭ��
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// ���ȼ�����
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// ��ֹ�ڴ浽�ڴ�Ĵ���
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);							// ����DMA1��4ͨ��

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel2, DISABLE);
}
// ***********************************************************
// ����3���� PB10:TX; PB11:RX
void USART3_Config(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	// Tx (PB.10)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// Rx (PB.11)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART3_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);

	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);
	USART_Cmd(USART3, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_USART3_RX_Config();

	DMA_USART3_TX_Config();
}

// ***********************************************************
// 1ms��ʱ��
void TIM2_Config()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	// TIM2 clock enable
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// TIM2CLK ��PCLK1=35MHz
	// TIM2CLK = 35 MHz, Prescaler = 70
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = (70 - 1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE);

	TIM_Cmd(TIM2, ENABLE); // ʹ��TIMx����

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			  // TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		  // �����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);
}
// ***********************************************************
// ���Ź�����6.4S����
void IWDG_Config(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256); // 6.4ms
	IWDG_SetReload(500);				   // 3.2Ss
	IWDG_ReloadCounter();
	IWDG_Enable();
}
// ***********************************************************
// ϵͳ����
void Sys_Config(void)
{
	GPIO_Config();

	USART1_Config();

	USART2_Config();

	USART3_Config();

	TIM2_Config();
}
