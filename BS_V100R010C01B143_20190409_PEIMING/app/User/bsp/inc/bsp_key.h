/*
*********************************************************************************************************
*
*	ģ������ : ��������ģ��
*	�ļ����� : bsp_key.h
*	��    �� : V1.0
*	˵    �� : ͷ�ļ�
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#include "stm32f10x.h"

#define RCC_APB_KEY 	 RCC_APB2Periph_GPIOF
#define GPIO_PORT_KEY  GPIOF
#define GPIO_PIN_KEY	 GPIO_Pin_10

#define KEY_EXTI_LINE        EXTI_Line10
#define KEY_EXTI_PortSource  GPIO_PortSourceGPIOF
#define KEY_EXTI_PinSource   GPIO_PinSource10

void bsp_Initkey(void);
void bsp_Initkey_Triggertype(EXTITrigger_TypeDef Trigger_TypeDef);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
