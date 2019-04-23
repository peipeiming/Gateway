#include "includes.h"

/*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	/* 
	  在启动调度前，为了防止初始化STM32外设时有中断服务程序执行，这里禁止全局中断(除了NMI和HardFault)。
	  这样做的好处是：
	  1. 防止执行的中断服务程序中有FreeRTOS的API函数。
	  2. 保证系统正常启动，不受别的中断影响。
	  3. 关于是否关闭全局中断，大家根据自己的实际情况设置即可。
	  在移植文件port.c中的函数prvStartFirstTask中会重新开启全局中断。通过指令cpsie i开启，__set_PRIMASK(1)
	  和cpsie i是等效的。
     */
	__set_PRIMASK(1); 	

  bsp_Init();  								/*硬件初始化 */
	
	app_flash_LoadSysConfig();  /*读取配置参数*/
	
	AppTaskCreate();            /*创建任务 */
	AppObjCreate();      				/*创建任务通信机制 */
  vTaskStartScheduler();      /*启动调度，开始执行任务*/
	/* 
	  如果系统正常启动是不会运行到这里的，运行到这里极有可能是用于定时器任务或者空闲任务的
	  heap空间不足造成创建失败，此要加大FreeRTOSConfig.h文件中定义的heap大小：
	*/
	while(1);
}
/*
*********************************************************************************************************
*	函 数 名: vTaskTaskWEB
*	功能说明: WEB 配置
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级: 1  
*********************************************************************************************************
*/
static void vTaskTaskWEB(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(1000);
		do_https();/*Web server测试程序*/;
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskCheckNet
*	功能说明: 检查网线连接状态 获取52832 ID 
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级: 1  
*********************************************************************************************************
*/
static void vTaskCheckNet(void *pvParameters)
{
	uint8_t tmp=0;
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 10 / portTICK_PERIOD_MS; /* 最大延迟100ms */
	
	while(1)
	{
		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* 事件标志组句柄 */
										 UPDATA_ALL_START,            		/* 等待TASK_UPDATDEVSTART_BIT被设置 */
										 pdFALSE,            							/* 退出前TASK_UPDATSTART_BIT不清除*/
										 pdTRUE,             		          /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
										 xTicksToWait); 								 	/* 等待延迟时间 */
		
		/*没有进入升级状态*/
		if(uxBits != UPDATA_GAT_START && uxBits != UPDATA_BLE_START && uxBits != UPDATA_BRA_START)
		{	
			/*检测网线连接状态*/
			ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);	
			
			if(tmp == PHY_LINK_ON && sysCfg.parameter.connect_state == CONNECT)//有网
			{ 
				app_system_NetLedToggle();		/*网关指示灯翻转*/
				do_ntp_client();	            /*获取网络时间*/
				vTaskDelay(1000);
				app_system_CheckID();         /*自动化系统获取设备ID  MCU ID + 蓝牙ID*/
				
				/*检查是否新的基站设备接入*/	
				if(sysCfg.parameter.register_flag == UNREGISTER)
				{
					app_nrf_GetNewStation();   
				}	
		    
				xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_NET);
			}
			
			else if(tmp == PHY_LINK_OFF)  
			{	
				xTimerStop(xPingTimers, 100); 								 /*关ping定时器*/
				
				bsp_LedOff(2); 
				bsp_LedOff(3);
				bsp_LedOn(4);
				
				sysCfg.parameter.connect_state = DISCONNECT;
				
				while(tmp == PHY_LINK_OFF)		/*循环检测网络*/
				{
					ctlwizchip(CW_GET_PHYLINK, (void*)&tmp); 
					xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_NET);
					vTaskDelay(1000);
					#if APP_DEBUG
					printf("Unknown PHY Link stauts.\r\n");
					#endif
				}

				app_system_MqttConnect(SOCK_TCPS);     				 /*连接到主服务器*/
				if(sysCfg.parameter.data_socket != SOCK_TCPS)  /*连接到第三方服务器*/
				{
					app_system_MqttConnect(SOCK_TCP);   
				}
				
				bsp_LedOff(4);
				xTimerStart(xPingTimers, 100);
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskTaskBleIF
*	功能说明: NRF串口数据处理
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级: 3  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
static void vTaskTaskBleIF(void *pvParameters)
{	
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 10 / portTICK_PERIOD_MS; /* 最大延迟100ms */
	
	while(1)
	{	
		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* 事件标志组句柄 */
										 UPDATA_ALL_START,            		/* 等待TASK_UPDATDEVSTART_BIT被设置 */
										 pdFALSE,            							/* 退出前TASK_UPDATSTART_BIT不清除*/
										 pdTRUE,             		          /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
										 xTicksToWait); 								 	/* 等待延迟时间 */
		
		/*没有进入升级状态*/
		if((uxBits != UPDATA_GAT_START) && (uxBits != UPDATA_BLE_START) && (uxBits !=UPDATA_BRA_START))
		{
			app_plat_SendMessage();     /*发送消息缓存中的消息*/
			
			if(sysCfg.parameter.register_flag == REGISTER)
			{
				app_nrf_GetBleData(sysCfg.parameter.nrfstation);
				vTaskDelay(20);  
			}
			app_nrf_DealBleData(); 
			
			vTaskDelay(10);
			
			/* 发送事件标志，表示任务正常运行 */
			xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_BLE);
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskStart
*	功能说明: 启动任务
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
* 优 先 级: 2  
*********************************************************************************************************
*/
static void vTaskStart(void *pvParameters)
{
	uint8_t rc;   
	uint8_t ucQueueValue;
	
	uint16_t wait_tcp_ack_time = 0;
	uint16_t wait_tcps_ack_time = 0;
	
	BaseType_t xResult;
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;  /* 最大延迟100ms */
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);			 /* 设置最大等待时间为100ms */	
	
	net_init();           /*网络初始化*/

  #if APP_DEBUG
	printf("Net init ok\r\n");
	#endif
	vTaskDelay(1000);	
	/*更新网络参数*/
	get_netparm();
	if(sysCfg.parameter.dhcp == NETINFO_STATIC)
	{
		rc = DHCP_IP_LEASED;
		set_netparm();
	}
	else
	{
		#if APP_DEBUG
		printf("Ready to run DHCP...\r\n");
		#endif
	}
		
	while(rc!=DHCP_IP_LEASED)
	{
	   rc=DHCP_run();	
		 IWDG_Feed();  /*喂狗*/
     vTaskDelay(2000); 
  }		
	
	/*更新网络参数*/
	get_netparm();
	#if APP_DEBUG
	printf("IP leased\r\n");
	printf("Ready to connect mqtt server...\r\n");
	#endif
	 
	app_system_Start();             /*连接与订阅*/
	xTaskCreate( vTaskCheckNet,"vTaskCheckNet",1024, NULL, 1, &xHandleCheckNet ); 		    
		 
	app_palt_fifoinit();            /*初始化信息存储FIFO*/
	app_nrf_ResetStation();         /*复位蓝牙*/
	vTaskDelay(1000);   
		
	while(1)                  
	{
		/* 等待所有任务发来事件标志 */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, 	/* 事件标志组句柄 */
										 TASK_BIT_ALL,       							/* 等待TASK_BIT_ALL被设置 */
										 pdTRUE,            							/* 退出前TASK_BIT_ALL被清除，这里是TASK_BIT_ALL都被设置才表示“退出”*/
										 pdTRUE,             							/* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
										 xTicksToWait); 								 	/* 等待延迟时间 */
	
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{
			IWDG_Feed();  /*喂狗*/
		}

		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* 事件标志组句柄 */
										 UPDATA_ALL_START,            		/* 等待TASK_UPDATDEVSTART_BIT被设置 */
										 pdFALSE,            							/* 退出前TASK_UPDATSTART_BIT不清除*/
										 pdTRUE,             		          /* 设置为pdTRUE表示等待TASK_BIT_ALL都被设置*/
										 xTicksToWait); 								 	/* 等待延迟时间 */
		
		/*没有进入升级状态*/
		if(uxBits != UPDATA_GAT_START && uxBits != UPDATA_BLE_START && uxBits !=UPDATA_BRA_START)
		{
			xResult = xQueueReceive(xQueue1,                    /* 消息队列句柄 */
															(void *)&ucQueueValue,      /* 存储接收到的数据到变量ucQueueMsgValue中 */
															(TickType_t)xMaxBlockTime); /* 设置阻塞时间 */
			if(xResult == pdPASS)  
			{
				if(ucQueueValue == 0)         /*收到主服务器的PING响应*/
				{
					wait_tcps_ack_time = 0;     /*清零等待时间*/
				}
				else if(ucQueueValue == 1)    /*收到第三方服务器的PING响应*/
				{
					wait_tcp_ack_time = 0;
				}
				
				#if APP_DEBUG
				printf("ucQueueValue:%d\r\n",ucQueueValue);
				#endif
			}														

			/*未PING 通服务器时间累加*/
			wait_tcp_ack_time++;
			wait_tcps_ack_time++;
			
			/*第三方服务器 PING 超出300秒无响应 重连*/
			if(wait_tcps_ack_time > 300)  
			{
				#if APP_DEBUG
				printf("ping tcps outtime\r\n");
				#endif
				wait_tcps_ack_time = 0;
				sysCfg.parameter.connect_state = DISCONNECT;
				app_system_MqttConnect(SOCK_TCPS);   
			}
			
			/*第三方服务器 PING 超出300秒无响应 重连*/
			if(wait_tcp_ack_time > 300)
			{
				wait_tcp_ack_time = 0;
				if(sysCfg.parameter.data_socket != SOCK_TCPS)
				{
				  #if APP_DEBUG
					printf("ping tcp outtime\r\n");
					#endif
					sysCfg.parameter.connect_state = DISCONNECT;
					app_system_MqttConnect(SOCK_TCP);  
				}
			}
		}
			
		vTaskDelay(1000);
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTaskMsgPro 最高优先级任务，初次启动会阻塞
*	功能说明: 信息处理,最高优先级任务，只要接收到服务器，触发中断，会立即运行此任务，
            解析数据类型 消息类型：订阅消息，连接确认，ping响应
*	形    参: pvParameters 是在创建该任务时传递的形参

*	返 回 值: 无
* 优 先 级: 4  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	plat_report_t report_t;
	
  while(1)
  {
		xResult = xSemaphoreTake(xSemaphore, portMAX_DELAY);	//阻塞，等待中断give二值信号，以解除阻塞 portMAX_DELAY 表示一直等待
		if(xResult == pdTRUE)	 /* 接收到同步信号 */  
		{
			/*解析平台数据*/
			if(SUCCESS == app_palt_Reportparse(&report_t))
			{
				if(report_t.socket != SOCK_BLE)
				{
					switch( report_t.evt_id )
					{
						case PUBLISH:   /*接收订阅主题数据*/
							app_palt_Receicedata(&report_t);
							break;
						
						case CONNACK:   /*连接回执*/
							app_palt_Connectack(&report_t);
							break;
						
						case PINGRESP:  /*到服务器ping回执*/
							#if APP_DEBUG
						  printf("ping ack:%d\r\n",report_t.socket);
						  #endif
						  xQueueSend(xQueue1, (void *) &report_t.socket,(TickType_t)10); 
							break;
						
						default:
							break;
					}	
					
					if(report_t.gateway_updata_flag == GAT_UPDATA_START)
					{
						app_system_LedOn();
						xTimerStop(xPingTimers, 100);                                   /*关闭ping*/
						report_t.gateway_updata_flag = GAT_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_GAT_START);	      /*阻塞其他任务*/
					}
					
					if(report_t.ble_updata_flag == BLE_UPDATA_START)
					{
						app_system_LedOn();
						comClearTxFifo(NRF_PORT);
						xTimerStop(xPingTimers, 100); 
						report_t.ble_updata_flag = BLE_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_BLE_START);				/*阻塞其他任务*/
					}
					
					if(report_t.bracelet_updata_flag == BRA_UPDATA_START)
					{
						app_system_LedOn();
						xTimerStop(xPingTimers, 100); 
						report_t.bracelet_updata_flag = BRA_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_BRA_START);				/*阻塞其他任务*/
					}
				}
				else
				{
					IWDG_Feed(); 
					app_system_UpdataBle();
				}	
		  }	
    }
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppTaskCreate
*	功能说明: 创建应用任务
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	xTaskCreate( vTaskTaskWEB,   			    /* 任务函数  */
                 "vTaskTaskWEB",     		/* 任务名 	*/
                 512,               		/* 任务栈大小，单位word，也就是4字节 */
                 NULL,              		/* 任务参数  */
                 0,                 	  /* 任务优先级*/
                 &xHandleTaskWEB );  /* 任务句柄  */
	
	xTaskCreate( vTaskTaskBleIF,   			  /* 任务函数  */
                 "vTaskUserIF",     		/* 任务名 	*/
                 1024,               		/* 任务栈大小，单位word，也就是4字节 */
                 NULL,              		/* 任务参数  */
                 2,                 	  /* 任务优先级*/
                 &xHandleTaskUserIF );  /* 任务句柄  */

		xTaskCreate( vTaskStart,     				/* 任务函数  */
								 "vTaskStart",   				/* 任务名    */
								 512,            				/* 任务栈大小，单位word，也就是4字节 */
								 NULL,           				/* 任务参数  */
								 3,              				/* 任务优先级*/
								 &xHandleTaskStart );   /* 任务句柄  */
	
	xTaskCreate( vTaskMsgPro,     				/* 任务函数  */
                 "vTaskMsgPro",   			/* 任务名    */
                 2048,           	      /* 任务栈大小，单位word，也就是4字节 */
                 NULL,           				/* 任务参数  */
                 4,               			/* 任务优先级*/
                 &xHandleTaskMsgPro );  /* 任务句柄  */
}

#ifndef MCBR03
/*******************************************************************************
* 函数名  : EXTI9_5_IRQHandler
* 描述    : 中断线5中断服务函数(W5500 INT引脚中断服务函数)
* 输入    : 无
* 返回值  : 无
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
		
  	/* 发送计数信号量*/
	  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	  /* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
#endif


/*******************************************************************************
* 函数名  : EXTI9_5_IRQHandler
* 描述    : 中断线5中断服务函数(W5500 INT引脚中断服务函数)
* 输入    : 无
* 返回值  : 无
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	#ifdef MCBR03
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{		
		EXTI_ClearITPendingBit(EXTI_Line11);
		
  	/* 发送计数信号量*/
	  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	  /* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
  #endif
	
	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line10);
   
		delay_ms(20);  /*防抖动*/

    if(0 == GPIO_ReadInputDataBit( GPIOF , GPIO_Pin_10 ))	     /*按键按下*/
		{			
			bsp_LedOn(4);
			key_timer_Start();                                       /*按下开始按键计时*/
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Rising);            /*检测上升沿触发*/
		}
		else if(1 == GPIO_ReadInputDataBit( GPIOF , GPIO_Pin_10 )) /*按键松开*/ 	
		{
			bsp_LedOff(4);
			key_timer_Stop();                                        /*关闭按键计时*/
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Falling);           /*检测下升沿触发*/
		}
	}		
}

/*按键定时器 100ms*/
void TIM2_IRQHandler(void)
{		
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		
		sysCfg.parameter.key_time_count++;           
		 
		if(sysCfg.parameter.key_time_count == 50)    /*长按5s*/
		{
	    sysCfg.parameter.config_hold_flag = 0xff;  /*恢复出厂设置*/
			sysCfg.parameter.dhcp=NETINFO_DHCP;/*默认为开启DHCP*/
			if(0 == bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN))
			{
				mqtt_disconnect();
				delay_ms(100);
				NVIC_SystemReset();		
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: AppObjCreate
*	功能说明: 创建任务通信机制
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	uint8_t i=0;
	const TickType_t  xTimerPer = 30000;
	/* 创建二值信号量，首次创建信号量计数值是0 */
	xSemaphore = xSemaphoreCreateBinary();
	if(xSemaphore == NULL)
	{
		printf("create xSemaphore fail\r\n");
		while(1);
	}
	/* 创建检测任务运行事件标志组*/
	xCreatedEventGroup = xEventGroupCreate();
	if(xCreatedEventGroup == NULL)
	{
		printf("create EventGroup fail\r\n");
		while(1);
	}	
	
	/*创建消息队列 消息队列长度为1*/
	xQueue1 = xQueueCreate(2, sizeof(uint8_t));
	if( xQueue1 == 0 )
	{
		printf("QueueCreate fail\r\n");
	}

	/* 创建升级事件标志组*/
	xUpdataEventGroup = xEventGroupCreate();
	if(xUpdataEventGroup == NULL)
	{
		printf("create xUpdataEventGroup fail\r\n");
		while(1);
	}	
	
	xPingTimers = xTimerCreate("Timer",   /* 定时器名字 */
									 xTimerPer,    				/* 定时器周期,单位时钟节拍 */
									 pdTRUE ,         		/* 周期性 */
									 (void *) i,      		/* 定时器ID */
									 vPingTimerCallback); /* 定时器回调函数 */

	if(xPingTimers == NULL)
	{
		printf("create xTimers fail\r\n");
		while(1);
	}

	if(!xTimerStart(xPingTimers, 100) == pdPASS)
	{
		printf("Start ping Timer fail!\r\n");
	}
}

/*
*********************************************************************************************************
*	函 数 名: vTimerCallback
*	功能说明: 定时器回调函数
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void vPingTimerCallback(xTimerHandle pxTimer)
{
	configASSERT(pxTimer);
	
	if( sysCfg.parameter.connect_state == CONNECT )
	{
		/*主服务器ping*/
		Heartbeat(SOCK_TCPS);    
	 
		/*第三方服务器ping*/
		if(sysCfg.parameter.data_socket != SOCK_TCPS)
		{
			vTaskDelay(1000);
			Heartbeat(SOCK_TCP);  
		}
	}
}


