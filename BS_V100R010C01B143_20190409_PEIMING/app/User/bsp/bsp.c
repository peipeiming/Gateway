/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32F1XX)
*	文件名称 : bsp.c
*	版    本 : V1.0
*	说    明 : 这是硬件底层驱动程序模块的主文件。主要提供 bsp_Init()函数供主程序调用。主程序的每个c文件可以在开
*			  头	添加 #include "bsp.h" 来包含所有的外设驱动模块。
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-03-01 armfly   正式发布
*
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化硬件设备。只需要调用一次。该函数配置CPU寄存器和外设的寄存器并初始化一些全局变量。
*			 全局变量。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
/*******************************************************************************
* 函数名  : W5500_NVIC_Configuration
* 描述    : W5500 接收引脚中断优先级设置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the PB5 Interrupt */
	#ifdef MCBR03
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	#else
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	#endif
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void W5500_GPIO_Configuration(void)
{
  #ifdef  MCBR03
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);  //使能GPIOG时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	/* W5500_INT引脚初始化配置(PB5) */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOG,GPIO_Pin_15);
	delay_ms(10);
	GPIO_SetBits(GPIOG,GPIO_Pin_15);
	
	/* Connect EXTI Line4 to PB5 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource11);

	/* PA4 as W5500 interrupt input */
	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	#else
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	/* W5500_INT引脚初始化配置(PB5) */	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
	delay_ms(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_6);
	
	/* Connect EXTI Line4 to PB5 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);

	/* PA4 as W5500 interrupt input */
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	#endif
}

void bsp_Init(void)
{
	 uint8_t *pBytes;
	 uint8_t temp;
	 uint32_t flag=0xFFFFFFFF;
	 RCC_ClocksTypeDef  clk;
	/*
		由于ST固件库的启动文件已经执行了CPU系统时钟的初始化，所以不必再次重复配置系统时钟。
		启动文件配置了CPU主时钟频率、内部Flash访问速度和可选的外部SRAM FSMC初始化。
		系统时钟缺省配置为72MHz，如果需要更改，可以修改 system_stm32f10x.c 文件
	*/
	
	/* 优先级分组设置为4，可配置0-15级抢占式优先级，0级子优先级，即不存在子优先级。*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  SCB->VTOR = FLASH_BASE | 0x8000;	
	bsp_InitUart();              /*初始化串口 */
	bsp_InitLed();	             /*初始LED指示灯端口 */
	bsp_Initkey();               /*配置按键初始化*/
	bsp_InitIwdg(0x0FFF);        /*26.208s看门狗异常复位时间*/
	SPI_Configuration();         /*SPI用于STM32与W5500通信*/
	W5500_NVIC_Configuration();
  W5500_GPIO_Configuration();  /*初始化中断引脚*/	
	Timer_Config();              /*初始化DNS所需要的定时器*/
	
	#ifdef MCBR03
  IIC_Init();				      		/*温湿度初始化*/    
  SHT3X_Init();	
	#endif
	
	/*写应用程序标志位*/
	if((bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&flag,4)))
	{
		#if APP_DEBUG
	  printf("\r\nclear flag Fail\r\n");
		#endif
	}	
//	SystemInit
	RCC_GetClocksFreq(&clk);
	printf("SYSCLK_Frequency:%dHz\r\n\r\n",clk.SYSCLK_Frequency);	
}
