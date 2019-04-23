#include "bsp.h"

/**
  * @brief  初始化定时器中断
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
  * @brief  初始化定时器 
  * @param  None
  * @retval None
  */
void Timer_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3|RCC_APB1Periph_TIM2, ENABLE); 

	TIM_DeInit(TIM3);                              					//复位TIM3定时器
	TIM_DeInit(TIM2);
	
	/* TIM3 configuration */
	TIM_TimeBaseStructure.TIM_Period = 200;        					// 100ms    
	TIM_TimeBaseStructure.TIM_Prescaler = 36000;    				// 分频36000        
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// 时钟分频 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//计数方向向上计数
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* Clear TIM3 update pending flag[清除TIM3溢出中断标志] */
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);

	/* Enable TIM3 Update interrupt [TIM3溢出中断允许]*/
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE); 
	/* TIM3计数器清零*/
	TIM3->CNT=0;
	/* TIM3 enable counter [允许TIM3计数]*/
	TIM_Cmd(TIM3, DISABLE);  
	/*Config interrupts*/


	/* TIM2 configuration   按键定时器*/
	TIM_TimeBaseStructure.TIM_Period = 1000-1;        //10 				 
	TIM_TimeBaseStructure.TIM_Prescaler = 6400-1;    	//104		  	  
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  		// 时钟分频 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//计数方向向上计数
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Clear TIM2 update pending flag[清除TIM2溢出中断标志] */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* Enable TIM2 Update interrupt [TIM2溢出中断允许]*/
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); 
	/* TIM2计数器清零*/
	TIM2->CNT=0;
	/* TIM3 enable counter [允许TIM2计数]*/
	TIM_Cmd(TIM2, DISABLE);  
	/*Config interrupts*/
		
	Timer_Interrupts_Config();
}



/**
  * @brief  清除定时器计数器寄存器计数值并启动定时器
  * @param  None
  * @retval None
  */
void Timer_Start(void)
{
	TIM3->CNT=0;//清除计数器寄存器的值，可以减小丢帧的情况
  /* Enable the TIM Counter */
	TIM_Cmd(TIM3, ENABLE); 
}

/**
  * @brief  停止定时器并清除定时器的计数值
  * @param  None
  * @retval None
  */
void Timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM3, DISABLE);
}

/**
  * @brief  清除定时器计数器寄存器计数值并启动定时器
  * @param  None
  * @retval None
  */
void key_timer_Start(void)
{
	TIM2->CNT=0;//清除计数器寄存器的值，可以减小丢帧的情况
  /* Enable the TIM Counter */
	TIM_Cmd(TIM2, ENABLE); 
}

/**
  * @brief  停止定时器并清除定时器的计数值
  * @param  None
  * @retval None
  */
void key_timer_Stop(void)
{ 
  /* Disable the TIM Counter */
	TIM_Cmd(TIM2, DISABLE);
}

