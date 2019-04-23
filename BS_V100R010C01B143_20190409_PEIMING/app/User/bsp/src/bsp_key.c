#include "bsp_key.h"
#include "stm32f10x.h"

/*
*********************************************************************************************************
 * 函数名：bsp_Initkey
 * 描述  ：按键初始化
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
 */
void bsp_Initkey(void)
{   
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;			
	
	/* 使能 GPIO 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB_KEY, ENABLE);

	/* 配置 输入模式 */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_KEY;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIO_PORT_KEY, &GPIO_InitStructure);

	/* 使能 AFIO 时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(KEY_EXTI_PortSource, KEY_EXTI_PinSource);

	EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	/* 上升沿触发 */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

  #ifndef MCBR03
	/* Enable and set EXTI13 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 	
  #endif	
}

void bsp_Initkey_Triggertype(EXTITrigger_TypeDef Trigger_TypeDef)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = Trigger_TypeDef;  	/* 上升沿触发 */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	
}