/*
*********************************************************************************************************
*
*	ģ������ : ͷ�ļ�����
*	�ļ����� : includes.h
*	��    �� : V1.0
*	˵    �� : ��ǰʹ��ͷ�ļ�����
*
*	�޸ļ�¼ :
*		�汾��    ����        ����     ˵��
*		V1.0    2015-08-02  Eric2013   �״η���
*
*	Copyright (C), 2015-2020, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/*
*********************************************************************************************************
*                                         ��׼��
*********************************************************************************************************
*/
#include  <string.h>
#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>
#include  <ctype.h>
#include  "utility.h"


/*
*********************************************************************************************************
*                                           OS
*********************************************************************************************************
*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"
#include "semphr.h"
#include "event_groups.h"


/*
*********************************************************************************************************
*                                           �궨��
*********************************************************************************************************
*/




/*
*********************************************************************************************************
*                                        APP / BSP
*********************************************************************************************************
*/
#include "bsp.h"

#include "app_Plat.h"
#include "app_flash.h"
#include "app_system.h"
#include "app_nrf52832.h"
#include "app_Plat_crclist.h"
#include "app_device_alarm.h"


/*
*********************************************************************************************************
*                                         MQTT��&W5500��
*********************************************************************************************************
*/
#include "MQTTPacket.h"
#include "transport.h"//MQTTֻ��Ҫ������һ��ͷ�ļ�����
#include "socket.h"	// TCP/IPֻ��Ҫ������һ��ͷ�ļ�
#include "dns.h"  //������������ʱû�õ�
#include "dhcp.h"
#include "mqtt.h"
#include "ntp.h"
#include "http_server.h"
#include "httputil.h"
/*
**********************************************************************************************************
											�궨��
**********************************************************************************************************
*/
#define TASK_BIT_NET	 (1 << 0)
#define TASK_BIT_BLE   (1 << 1)
#define TASK_BIT_ALL (TASK_BIT_NET | TASK_BIT_BLE)

#define UPDATA_GAT_START (1 << 0)
#define UPDATA_BLE_START (1 << 1)
#define UPDATA_BRA_START (1 << 2)
#define UPDATA_ALL_START (UPDATA_GAT_START | UPDATA_BLE_START | UPDATA_BRA_START)

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/
static void vTaskLED(void *pvParameters);
static void vTaskStart(void *pvParameters);
static void vTaskMsgPro(void *pvParameters);
static void vTaskTaskUserIF(void *pvParameters);
static void AppObjCreate (void);
static void AppTaskCreate (void);
static void vPingTimerCallback(xTimerHandle pxTimer);
static void vNoNetTimerCallback(xTimerHandle pxTimer);

/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/

static TaskHandle_t xHandleTaskUserIF = NULL;
static TaskHandle_t xHandleCheckNet = NULL;
static TaskHandle_t xHandleTaskWEB = NULL;
static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleTaskStart = NULL;
static SemaphoreHandle_t xSemaphore = NULL;
static SemaphoreHandle_t xMutex = NULL;
static TimerHandle_t xPingTimers=NULL;
static TimerHandle_t xNoNetTimers=NULL;
static EventGroupHandle_t xCreatedEventGroup = NULL;
static EventGroupHandle_t xUpdataEventGroup = NULL;
static QueueHandle_t xQueue1 = NULL;

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
