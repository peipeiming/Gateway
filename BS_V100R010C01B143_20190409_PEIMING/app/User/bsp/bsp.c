/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��(For STM32F1XX)
*	�ļ����� : bsp.c
*	��    �� : V1.0
*	˵    �� : ����Ӳ���ײ���������ģ������ļ�����Ҫ�ṩ bsp_Init()��������������á��������ÿ��c�ļ������ڿ�
*			  ͷ	��� #include "bsp.h" ���������е���������ģ�顣
*
*	�޸ļ�¼ :
*		�汾��  ����        ����     ˵��
*		V1.0    2013-03-01 armfly   ��ʽ����
*
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
/*
*********************************************************************************************************
*	�� �� ��: bsp_Init
*	����˵��: ��ʼ��Ӳ���豸��ֻ��Ҫ����һ�Ρ��ú�������CPU�Ĵ���������ļĴ�������ʼ��һЩȫ�ֱ�����
*			 ȫ�ֱ�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
/*******************************************************************************
* ������  : W5500_NVIC_Configuration
* ����    : W5500 ���������ж����ȼ�����
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
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
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);  //ʹ��GPIOGʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	/* W5500_INT���ų�ʼ������(PB5) */	
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
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOBʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 
	/* W5500_INT���ų�ʼ������(PB5) */	
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
		����ST�̼���������ļ��Ѿ�ִ����CPUϵͳʱ�ӵĳ�ʼ�������Բ����ٴ��ظ�����ϵͳʱ�ӡ�
		�����ļ�������CPU��ʱ��Ƶ�ʡ��ڲ�Flash�����ٶȺͿ�ѡ���ⲿSRAM FSMC��ʼ����
		ϵͳʱ��ȱʡ����Ϊ72MHz�������Ҫ���ģ������޸� system_stm32f10x.c �ļ�
	*/
	
	/* ���ȼ���������Ϊ4��������0-15����ռʽ���ȼ���0�������ȼ����������������ȼ���*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
  SCB->VTOR = FLASH_BASE | 0x8000;	
	bsp_InitUart();              /*��ʼ������ */
	bsp_InitLed();	             /*��ʼLEDָʾ�ƶ˿� */
	bsp_Initkey();               /*���ð�����ʼ��*/
	bsp_InitIwdg(0x0FFF);        /*26.208s���Ź��쳣��λʱ��*/
	SPI_Configuration();         /*SPI����STM32��W5500ͨ��*/
	W5500_NVIC_Configuration();
  W5500_GPIO_Configuration();  /*��ʼ���ж�����*/	
	Timer_Config();              /*��ʼ��DNS����Ҫ�Ķ�ʱ��*/
	
	#ifdef MCBR03
  IIC_Init();				      		/*��ʪ�ȳ�ʼ��*/    
  SHT3X_Init();	
	#endif
	
	/*дӦ�ó����־λ*/
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
