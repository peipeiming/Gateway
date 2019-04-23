#include "bsp.h"

/**
  * @brief  ��ʼ����ʱ���ж�
  * @param  None
  * @retval None
  */
void Timer_Interrupts_Config(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 8;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/**
  * @brief  ��ʼ����ʱ�� 
  * @param  None
  * @retval None
  */
void Timer_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM2, ENABLE); 

	TIM_DeInit(TIM3);                              					//��λTIM3��ʱ��
	TIM_DeInit(TIM2);
	
	/* TIM3 configuration */
	TIM_TimeBaseStructure.TIM_Period = 200;        					// 100ms    
	TIM_TimeBaseStructure.TIM_Prescaler = 36000;    				// ��Ƶ36000        
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// ʱ�ӷ�Ƶ 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//�����������ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Clear TIM3 update pending flag[���TIM3����жϱ�־] */
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

	/* Enable TIM3 Update interrupt [TIM3����ж�����]*/
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
	/* TIM3����������*/
	TIM3->CNT=0;
	/* TIM3 enable counter [����TIM3����]*/
	TIM_Cmd(TIM3, DISABLE);  
	/*Config interrupts*/


	/* TIM2 configuration   ������ʱ��*/
	TIM_TimeBaseStructure.TIM_Period = 1000-1;        //10 				 
	TIM_TimeBaseStructure.TIM_Prescaler = 6400-1;    	//104		  	  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// ʱ�ӷ�Ƶ 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//�����������ϼ���
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Clear TIM2 update pending flag[���TIM2����жϱ�־] */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* Enable TIM2 Update interrupt [TIM2����ж�����]*/
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); 
	/* TIM2����������*/
	TIM2->CNT=0;
	/* TIM3 enable counter [����TIM2����]*/
	TIM_Cmd(TIM2, DISABLE);  
	/*Config interrupts*/
		
	Timer_Interrupts_Config();
}



/**
  * @brief  �����ʱ���������Ĵ�������ֵ��������ʱ��
  * @param  None
  * @retval None
  */
void Timer_Start(void)
{
	TIM3->CNT=0;//����������Ĵ�����ֵ�����Լ�С��֡�����
  /* Enable the TIM Counter */
	TIM_Cmd(TIM3, ENABLE); 
}

/**
  * @brief  ֹͣ��ʱ���������ʱ���ļ���ֵ
  * @param  None
  * @retval None
  */
void Timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM3, DISABLE);
}

/**
  * @brief  �����ʱ���������Ĵ�������ֵ��������ʱ��
  * @param  None
  * @retval None
  */
void key_timer_Start(void)
{
	TIM2->CNT=0;//����������Ĵ�����ֵ�����Լ�С��֡�����
  /* Enable the TIM Counter */
	TIM_Cmd(TIM2, ENABLE); 
}

/**
  * @brief  ֹͣ��ʱ���������ʱ���ļ���ֵ
  * @param  None
  * @retval None
  */
void key_timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM2, DISABLE);
}

