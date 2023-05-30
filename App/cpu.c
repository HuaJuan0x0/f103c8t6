
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

		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable); // 使能flash预读取缓冲区

		// 令Flash处于等待状态，2是针对高频时钟的，这两句跟RCC没直接关系，可以暂且略过
		FLASH_SetLatency(FLASH_Latency_2);
		RCC_HCLKConfig(RCC_SYSCLK_Div1);  // HCLK = SYSCLK 设置高速总线时钟=系统时钟
		RCC_PCLK2Config(RCC_HCLK_Div1);	  // PCLK2 = HCLK 设置低速总线2时钟=高速总线时钟
		RCC_PCLK1Config(RCC_HCLK_Div2);	  // PCLK1 = HCLK/2 设置低速总线1的时钟=高速时钟的二分频
		RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADCCLK = PCLK2/6 设置ADC外设时钟=低速总线2时钟的六分频
		// Set PLL clock output to 72MHz using HSE (8MHz) as entry clock
		// 上面这句例程中缺失了，但却很关键
		// 利用锁相环将外部10Mhz晶振7倍频到70Mhz
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_7);
		RCC_PLLCmd(ENABLE); // Enable PLL 使能锁相环

		// Wait till PLL is ready 等待锁相环输出稳定
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
			;

		// Select PLL as system clock source 将锁相环输出设置为系统时钟
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// Wait till PLL is used as system clock source 等待校验成功
		while (RCC_GetSYSCLKSource() != 0x08)
			;
	}
}
// ***********************************************************
// GPIO初始化
// 串口1：30	PA9/TX1 	与嵌入式主机通信
//       31		PA10/RX1
// 串口2：12	PA2/TX2		无使用
// 		 13		PA3/RX2		接受GNSS_GGA	MCU授时
// 串口3：21	PB10/TX3	串口，对外预留口	RS232电平
// 		 22		PB11/RX3

// 		 19		PB1			PPS接入
//		 14		PA4			触发IO	与相机设置配合
//		 15		PA5			LED	IO低-亮，高-灭
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
	// 串口1：30	PA9/TX1 	与嵌入式主机通信
	//       31		PA10/RX1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 串口2：12	PA2/TX2		无使用
	// 		 13		PA3/RX2		接受GNSS_GGA	MCU授时
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// 串口3：21	PB10/TX3	串口，对外预留口	RS232电平
	// 		 22		PB11/RX3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	// PPS PB1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	// TODO: gpio_mode 不清楚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	//		 14		PA4			触发IO	与相机设置配合
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//		 15		PA5			LED	IO低-亮，高-灭
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
// ***********************************************************
// 串口1接收的DMA设置，DMA1，通道5
void DMA_USART1_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel5);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart1RxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// 方向(从外设到内存)
	DMA_InitStructure.DMA_BufferSize = g_Uart1RxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);							// 配置DMA1的5通道

	DMA_Cmd(DMA1_Channel5, ENABLE);
}

// ***********************************************************
// 串口1传输的DMA设置，DMA1，通道4
void DMA_USART1_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart1TxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// 方向(从内存到外设)
	DMA_InitStructure.DMA_BufferSize = g_Uart1TxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);							// 配置DMA1的4通道

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel4, DISABLE);
}
// ***********************************************************
// 串口1配置 PA9:TX; PA10:RX
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
	// TODO: gpio 模式
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
// 串口2接收的DMA设置，DMA1，通道7
void DMA_USART2_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart2RxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// 方向(从外设到内存)
	DMA_InitStructure.DMA_BufferSize = g_Uart2RxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);							// 配置DMA1的4通道

	DMA_Cmd(DMA1_Channel7, ENABLE);
}

// ***********************************************************
// 串口2接收的DMA设置，DMA1，通道6
void DMA_USART2_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART2->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart2TxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// 方向(从内存到外设)
	DMA_InitStructure.DMA_BufferSize = g_Uart2TxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);							// 配置DMA1的4通道

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel6, DISABLE);
}
// ***********************************************************
// 串口2配置 PA2:TX; PA3:RX
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
// 串口3接收的DMA设置，DMA1，通道3
void DMA_USART3_RX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel3);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart3RxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;						// 方向(从外设到内存)
	DMA_InitStructure.DMA_BufferSize = g_Uart3RxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);							// 配置DMA1的4通道

	DMA_Cmd(DMA1_Channel3, ENABLE);
}

// ***********************************************************
// 串口3接收的DMA设置，DMA1，通道2
void DMA_USART3_TX_Config()
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA配置

	DMA_DeInit(DMA1_Channel2);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART3->DR);		// 串口数据寄存器地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)g_Uart3TxBuf.pDMABuf;	// 内存地址(要传输的变量的指针)
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;						// 方向(从内存到外设)
	DMA_InitStructure.DMA_BufferSize = g_Uart3TxBuf.nDMASize;				// 传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		// 外设地址不增
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					// 内存地址自增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // 外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			// 内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							// DMA模式：一次传输，循环
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					// 优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;							// 禁止内存到内存的传输
	DMA_Init(DMA1_Channel2, &DMA_InitStructure);							// 配置DMA1的4通道

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel2, DISABLE);
}
// ***********************************************************
// 串口3配置 PB10:TX; PB11:RX
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
// 1ms定时器
void TIM2_Config()
{
	NVIC_InitTypeDef NVIC_InitStructure;

	// TIM2 clock enable
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	// TIM2CLK 即PCLK1=35MHz
	// TIM2CLK = 35 MHz, Prescaler = 70
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = (70 - 1);
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ITConfig(TIM2, TIM_IT_Update | TIM_IT_Trigger, ENABLE);

	TIM_Cmd(TIM2, ENABLE); // 使能TIMx外设

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;			  // TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;		  // 从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);
}
// ***********************************************************
// 看门狗设置6.4S重启
void IWDG_Config(void)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256); // 6.4ms
	IWDG_SetReload(500);				   // 3.2Ss
	IWDG_ReloadCounter();
	IWDG_Enable();
}
// ***********************************************************
// 系统配置
void Sys_Config(void)
{
	GPIO_Config();

	USART1_Config();

	USART2_Config();

	USART3_Config();

	TIM2_Config();
}
