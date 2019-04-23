#include "includes.h"

/*********************************************************************************************************
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
	/* 
	  ����������ǰ��Ϊ�˷�ֹ��ʼ��STM32����ʱ���жϷ������ִ�У������ֹȫ���ж�(����NMI��HardFault)��
	  �������ĺô��ǣ�
	  1. ��ִֹ�е��жϷ����������FreeRTOS��API������
	  2. ��֤ϵͳ�������������ܱ���ж�Ӱ�졣
	  3. �����Ƿ�ر�ȫ���жϣ���Ҹ����Լ���ʵ��������ü��ɡ�
	  ����ֲ�ļ�port.c�еĺ���prvStartFirstTask�л����¿���ȫ���жϡ�ͨ��ָ��cpsie i������__set_PRIMASK(1)
	  ��cpsie i�ǵ�Ч�ġ�
     */
	__set_PRIMASK(1); 	

  bsp_Init();  								/*Ӳ����ʼ�� */
	
	app_flash_LoadSysConfig();  /*��ȡ���ò���*/
	
	AppTaskCreate();            /*�������� */
	AppObjCreate();      				/*��������ͨ�Ż��� */
  vTaskStartScheduler();      /*�������ȣ���ʼִ������*/
	/* 
	  ���ϵͳ���������ǲ������е�����ģ����е����Ｋ�п��������ڶ�ʱ��������߿��������
	  heap�ռ䲻����ɴ���ʧ�ܣ���Ҫ�Ӵ�FreeRTOSConfig.h�ļ��ж����heap��С��
	*/
	while(1);
}
/*
*********************************************************************************************************
*	�� �� ��: vTaskTaskWEB
*	����˵��: WEB ����
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 1  
*********************************************************************************************************
*/
static void vTaskTaskWEB(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(1000);
		do_https();/*Web server���Գ���*/;
	}
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskCheckNet
*	����˵��: �����������״̬ ��ȡ52832 ID 
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 1  
*********************************************************************************************************
*/
static void vTaskCheckNet(void *pvParameters)
{
	uint8_t tmp=0;
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 10 / portTICK_PERIOD_MS; /* ����ӳ�100ms */
	
	while(1)
	{
		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* �¼���־���� */
										 UPDATA_ALL_START,            		/* �ȴ�TASK_UPDATDEVSTART_BIT������ */
										 pdFALSE,            							/* �˳�ǰTASK_UPDATSTART_BIT�����*/
										 pdTRUE,             		          /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
										 xTicksToWait); 								 	/* �ȴ��ӳ�ʱ�� */
		
		/*û�н�������״̬*/
		if(uxBits != UPDATA_GAT_START && uxBits != UPDATA_BLE_START && uxBits != UPDATA_BRA_START)
		{	
			/*�����������״̬*/
			ctlwizchip(CW_GET_PHYLINK, (void*)&tmp);	
			
			if(tmp == PHY_LINK_ON && sysCfg.parameter.connect_state == CONNECT)//����
			{ 
				app_system_NetLedToggle();		/*����ָʾ�Ʒ�ת*/
				do_ntp_client();	            /*��ȡ����ʱ��*/
				vTaskDelay(1000);
				app_system_CheckID();         /*�Զ���ϵͳ��ȡ�豸ID  MCU ID + ����ID*/
				
				/*����Ƿ��µĻ�վ�豸����*/	
				if(sysCfg.parameter.register_flag == UNREGISTER)
				{
					app_nrf_GetNewStation();   
				}	
		    
				xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_NET);
			}
			
			else if(tmp == PHY_LINK_OFF)  
			{	
				xTimerStop(xPingTimers, 100); 								 /*��ping��ʱ��*/
				
				bsp_LedOff(2); 
				bsp_LedOff(3);
				bsp_LedOn(4);
				
				sysCfg.parameter.connect_state = DISCONNECT;
				
				while(tmp == PHY_LINK_OFF)		/*ѭ���������*/
				{
					ctlwizchip(CW_GET_PHYLINK, (void*)&tmp); 
					xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_NET);
					vTaskDelay(1000);
					#if APP_DEBUG
					printf("Unknown PHY Link stauts.\r\n");
					#endif
				}

				app_system_MqttConnect(SOCK_TCPS);     				 /*���ӵ���������*/
				if(sysCfg.parameter.data_socket != SOCK_TCPS)  /*���ӵ�������������*/
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
*	�� �� ��: vTaskTaskBleIF
*	����˵��: NRF�������ݴ���
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 3  (��ֵԽС���ȼ�Խ�ͣ������uCOS�෴)
*********************************************************************************************************
*/
static void vTaskTaskBleIF(void *pvParameters)
{	
	EventBits_t uxBits;
	const TickType_t xTicksToWait = 10 / portTICK_PERIOD_MS; /* ����ӳ�100ms */
	
	while(1)
	{	
		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* �¼���־���� */
										 UPDATA_ALL_START,            		/* �ȴ�TASK_UPDATDEVSTART_BIT������ */
										 pdFALSE,            							/* �˳�ǰTASK_UPDATSTART_BIT�����*/
										 pdTRUE,             		          /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
										 xTicksToWait); 								 	/* �ȴ��ӳ�ʱ�� */
		
		/*û�н�������״̬*/
		if((uxBits != UPDATA_GAT_START) && (uxBits != UPDATA_BLE_START) && (uxBits !=UPDATA_BRA_START))
		{
			app_plat_SendMessage();     /*������Ϣ�����е���Ϣ*/
			
			if(sysCfg.parameter.register_flag == REGISTER)
			{
				app_nrf_GetBleData(sysCfg.parameter.nrfstation);
				vTaskDelay(20);  
			}
			app_nrf_DealBleData(); 
			
			vTaskDelay(10);
			
			/* �����¼���־����ʾ������������ */
			xEventGroupSetBits(xCreatedEventGroup, TASK_BIT_BLE);
		}
	}
}

/*
*********************************************************************************************************
*	�� �� ��: vTaskStart
*	����˵��: ��������
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
* �� �� ��: 2  
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
	const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;  /* ����ӳ�100ms */
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(100);			 /* �������ȴ�ʱ��Ϊ100ms */	
	
	net_init();           /*�����ʼ��*/

  #if APP_DEBUG
	printf("Net init ok\r\n");
	#endif
	vTaskDelay(1000);	
	/*�����������*/
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
		 IWDG_Feed();  /*ι��*/
     vTaskDelay(2000); 
  }		
	
	/*�����������*/
	get_netparm();
	#if APP_DEBUG
	printf("IP leased\r\n");
	printf("Ready to connect mqtt server...\r\n");
	#endif
	 
	app_system_Start();             /*�����붩��*/
	xTaskCreate( vTaskCheckNet,"vTaskCheckNet",1024, NULL, 1, &xHandleCheckNet ); 		    
		 
	app_palt_fifoinit();            /*��ʼ����Ϣ�洢FIFO*/
	app_nrf_ResetStation();         /*��λ����*/
	vTaskDelay(1000);   
		
	while(1)                  
	{
		/* �ȴ������������¼���־ */
		uxBits = xEventGroupWaitBits(xCreatedEventGroup, 	/* �¼���־���� */
										 TASK_BIT_ALL,       							/* �ȴ�TASK_BIT_ALL������ */
										 pdTRUE,            							/* �˳�ǰTASK_BIT_ALL�������������TASK_BIT_ALL�������òű�ʾ���˳���*/
										 pdTRUE,             							/* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
										 xTicksToWait); 								 	/* �ȴ��ӳ�ʱ�� */
	
		if((uxBits & TASK_BIT_ALL) == TASK_BIT_ALL)
		{
			IWDG_Feed();  /*ι��*/
		}

		uxBits = xEventGroupWaitBits(xUpdataEventGroup, 	/* �¼���־���� */
										 UPDATA_ALL_START,            		/* �ȴ�TASK_UPDATDEVSTART_BIT������ */
										 pdFALSE,            							/* �˳�ǰTASK_UPDATSTART_BIT�����*/
										 pdTRUE,             		          /* ����ΪpdTRUE��ʾ�ȴ�TASK_BIT_ALL��������*/
										 xTicksToWait); 								 	/* �ȴ��ӳ�ʱ�� */
		
		/*û�н�������״̬*/
		if(uxBits != UPDATA_GAT_START && uxBits != UPDATA_BLE_START && uxBits !=UPDATA_BRA_START)
		{
			xResult = xQueueReceive(xQueue1,                    /* ��Ϣ���о�� */
															(void *)&ucQueueValue,      /* �洢���յ������ݵ�����ucQueueMsgValue�� */
															(TickType_t)xMaxBlockTime); /* ��������ʱ�� */
			if(xResult == pdPASS)  
			{
				if(ucQueueValue == 0)         /*�յ�����������PING��Ӧ*/
				{
					wait_tcps_ack_time = 0;     /*����ȴ�ʱ��*/
				}
				else if(ucQueueValue == 1)    /*�յ���������������PING��Ӧ*/
				{
					wait_tcp_ack_time = 0;
				}
				
				#if APP_DEBUG
				printf("ucQueueValue:%d\r\n",ucQueueValue);
				#endif
			}														

			/*δPING ͨ������ʱ���ۼ�*/
			wait_tcp_ack_time++;
			wait_tcps_ack_time++;
			
			/*������������ PING ����300������Ӧ ����*/
			if(wait_tcps_ack_time > 300)  
			{
				#if APP_DEBUG
				printf("ping tcps outtime\r\n");
				#endif
				wait_tcps_ack_time = 0;
				sysCfg.parameter.connect_state = DISCONNECT;
				app_system_MqttConnect(SOCK_TCPS);   
			}
			
			/*������������ PING ����300������Ӧ ����*/
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
*	�� �� ��: vTaskMsgPro ������ȼ����񣬳�������������
*	����˵��: ��Ϣ����,������ȼ�����ֻҪ���յ��������������жϣ����������д�����
            ������������ ��Ϣ���ͣ�������Ϣ������ȷ�ϣ�ping��Ӧ
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�

*	�� �� ֵ: ��
* �� �� ��: 4  
*********************************************************************************************************
*/
static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	plat_report_t report_t;
	
  while(1)
  {
		xResult = xSemaphoreTake(xSemaphore, portMAX_DELAY);	//�������ȴ��ж�give��ֵ�źţ��Խ������ portMAX_DELAY ��ʾһֱ�ȴ�
		if(xResult == pdTRUE)	 /* ���յ�ͬ���ź� */  
		{
			/*����ƽ̨����*/
			if(SUCCESS == app_palt_Reportparse(&report_t))
			{
				if(report_t.socket != SOCK_BLE)
				{
					switch( report_t.evt_id )
					{
						case PUBLISH:   /*���ն�����������*/
							app_palt_Receicedata(&report_t);
							break;
						
						case CONNACK:   /*���ӻ�ִ*/
							app_palt_Connectack(&report_t);
							break;
						
						case PINGRESP:  /*��������ping��ִ*/
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
						xTimerStop(xPingTimers, 100);                                   /*�ر�ping*/
						report_t.gateway_updata_flag = GAT_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_GAT_START);	      /*������������*/
					}
					
					if(report_t.ble_updata_flag == BLE_UPDATA_START)
					{
						app_system_LedOn();
						comClearTxFifo(NRF_PORT);
						xTimerStop(xPingTimers, 100); 
						report_t.ble_updata_flag = BLE_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_BLE_START);				/*������������*/
					}
					
					if(report_t.bracelet_updata_flag == BRA_UPDATA_START)
					{
						app_system_LedOn();
						xTimerStop(xPingTimers, 100); 
						report_t.bracelet_updata_flag = BRA_UPDATA_RESET;
						xEventGroupSetBits(xUpdataEventGroup, UPDATA_BRA_START);				/*������������*/
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
*	�� �� ��: AppTaskCreate
*	����˵��: ����Ӧ������
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppTaskCreate (void)
{
	xTaskCreate( vTaskTaskWEB,   			    /* ������  */
                 "vTaskTaskWEB",     		/* ������ 	*/
                 512,               		/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              		/* �������  */
                 0,                 	  /* �������ȼ�*/
                 &xHandleTaskWEB );  /* ������  */
	
	xTaskCreate( vTaskTaskBleIF,   			  /* ������  */
                 "vTaskUserIF",     		/* ������ 	*/
                 1024,               		/* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,              		/* �������  */
                 2,                 	  /* �������ȼ�*/
                 &xHandleTaskUserIF );  /* ������  */

		xTaskCreate( vTaskStart,     				/* ������  */
								 "vTaskStart",   				/* ������    */
								 512,            				/* ����ջ��С����λword��Ҳ����4�ֽ� */
								 NULL,           				/* �������  */
								 3,              				/* �������ȼ�*/
								 &xHandleTaskStart );   /* ������  */
	
	xTaskCreate( vTaskMsgPro,     				/* ������  */
                 "vTaskMsgPro",   			/* ������    */
                 2048,           	      /* ����ջ��С����λword��Ҳ����4�ֽ� */
                 NULL,           				/* �������  */
                 4,               			/* �������ȼ�*/
                 &xHandleTaskMsgPro );  /* ������  */
}

#ifndef MCBR03
/*******************************************************************************
* ������  : EXTI9_5_IRQHandler
* ����    : �ж���5�жϷ�����(W5500 INT�����жϷ�����)
* ����    : ��
* ����ֵ  : ��
*******************************************************************************/
void EXTI9_5_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
		
  	/* ���ͼ����ź���*/
	  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	  /* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}
#endif


/*******************************************************************************
* ������  : EXTI9_5_IRQHandler
* ����    : �ж���5�жϷ�����(W5500 INT�����жϷ�����)
* ����    : ��
* ����ֵ  : ��
*******************************************************************************/
void EXTI15_10_IRQHandler(void)
{
	#ifdef MCBR03
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(EXTI_GetITStatus(EXTI_Line11) != RESET)
	{		
		EXTI_ClearITPendingBit(EXTI_Line11);
		
  	/* ���ͼ����ź���*/
	  xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	  /* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
	  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
  #endif
	
	if(EXTI_GetITStatus(EXTI_Line10) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line10);
   
		delay_ms(20);  /*������*/

    if(0 == GPIO_ReadInputDataBit( GPIOF , GPIO_Pin_10 ))	     /*��������*/
		{			
			bsp_LedOn(4);
			key_timer_Start();                                       /*���¿�ʼ������ʱ*/
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Rising);            /*��������ش���*/
		}
		else if(1 == GPIO_ReadInputDataBit( GPIOF , GPIO_Pin_10 )) /*�����ɿ�*/ 	
		{
			bsp_LedOff(4);
			key_timer_Stop();                                        /*�رհ�����ʱ*/
			sysCfg.parameter.key_time_count = 0;
			bsp_Initkey_Triggertype(EXTI_Trigger_Falling);           /*��������ش���*/
		}
	}		
}

/*������ʱ�� 100ms*/
void TIM2_IRQHandler(void)
{		
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		
		sysCfg.parameter.key_time_count++;           
		 
		if(sysCfg.parameter.key_time_count == 50)    /*����5s*/
		{
	    sysCfg.parameter.config_hold_flag = 0xff;  /*�ָ���������*/
			sysCfg.parameter.dhcp=NETINFO_DHCP;/*Ĭ��Ϊ����DHCP*/
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
*	�� �� ��: AppObjCreate
*	����˵��: ��������ͨ�Ż���
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void AppObjCreate (void)
{
	uint8_t i=0;
	const TickType_t  xTimerPer = 30000;
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	xSemaphore = xSemaphoreCreateBinary();
	if(xSemaphore == NULL)
	{
		printf("create xSemaphore fail\r\n");
		while(1);
	}
	/* ����������������¼���־��*/
	xCreatedEventGroup = xEventGroupCreate();
	if(xCreatedEventGroup == NULL)
	{
		printf("create EventGroup fail\r\n");
		while(1);
	}	
	
	/*������Ϣ���� ��Ϣ���г���Ϊ1*/
	xQueue1 = xQueueCreate(2, sizeof(uint8_t));
	if( xQueue1 == 0 )
	{
		printf("QueueCreate fail\r\n");
	}

	/* ���������¼���־��*/
	xUpdataEventGroup = xEventGroupCreate();
	if(xUpdataEventGroup == NULL)
	{
		printf("create xUpdataEventGroup fail\r\n");
		while(1);
	}	
	
	xPingTimers = xTimerCreate("Timer",   /* ��ʱ������ */
									 xTimerPer,    				/* ��ʱ������,��λʱ�ӽ��� */
									 pdTRUE ,         		/* ������ */
									 (void *) i,      		/* ��ʱ��ID */
									 vPingTimerCallback); /* ��ʱ���ص����� */

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
*	�� �� ��: vTimerCallback
*	����˵��: ��ʱ���ص�����
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/
static void vPingTimerCallback(xTimerHandle pxTimer)
{
	configASSERT(pxTimer);
	
	if( sysCfg.parameter.connect_state == CONNECT )
	{
		/*��������ping*/
		Heartbeat(SOCK_TCPS);    
	 
		/*������������ping*/
		if(sysCfg.parameter.data_socket != SOCK_TCPS)
		{
			vTaskDelay(1000);
			Heartbeat(SOCK_TCP);  
		}
	}
}


