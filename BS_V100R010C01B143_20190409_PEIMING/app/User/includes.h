/*
*********************************************************************************************************
*
*	模块名称 : 头文件汇总
*	文件名称 : includes.h
*	版    本 : V1.0
*	说    明 : 当前使用头文件汇总
*
*	修改记录 :
*		版本号    日期        作者     说明
*		V1.0    2015-08-02  Eric2013   首次发布
*
*	Copyright (C), 2015-2020, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef  __INCLUDES_H__
#define  __INCLUDES_H__

/*
*********************************************************************************************************
*                                         标准库
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
*                                           宏定义
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
*                                         MQTT库&W5500库
*********************************************************************************************************
*/
#include "MQTTPacket.h"
#include "transport.h"//MQTT只需要包含这一个头文件即可
#include "socket.h"	// TCP/IP只需要包含着一个头文件
#include "dns.h"  //域名解析，暂时没用到
#include "dhcp.h"
#include "mqtt.h"
#include "ntp.h"
#include "http_server.h"
#include "httputil.h"
/*
**********************************************************************************************************
											宏定义
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
											函数声明
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
											变量声明
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

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
