#include "bsp_key.h"
#include "stm32f10x.h"

/*
*********************************************************************************************************
 * ��������bsp_Initkey
 * ����  ��������ʼ��
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void bsp_Initkey(void)
{   
	GPIO_InitTypeDef   GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;			
	
	/* ʹ�� GPIO ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB_KEY, ENABLE);

	/* ���� ����ģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_PIN_KEY;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIO_PORT_KEY, &GPIO_InitStructure);

	/* ʹ�� AFIO ʱ�� */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(KEY_EXTI_PortSource, KEY_EXTI_PinSource);

	EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  	/* �����ش��� */
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
	EXTI_InitStructure.EXTI_Trigger = Trigger_TypeDef;  	/* �����ش��� */
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);	
}