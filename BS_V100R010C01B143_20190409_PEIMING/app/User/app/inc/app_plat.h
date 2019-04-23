#ifndef __APP_PLAT_H
#define __APP_PLAT_H

#include "stm32f10x.h"
#include "MQTTPacket.h"

/***************************发布主题*****************************/
#define HIS_SPORT_TOPIC    "/bracelet/report/history_sport_data"/*历史运动数据主题*/
#define SPORT_TOPIC        "/bracelet/report/sport_data"     	  /*运动数据主题*/
#define HEALTH_TOPIC       "/bracelet/report/heartrate_data" 	  /*健康数据主题*/
#define LOCATION_TOPIC     "/bracelet/report/location"        	/*位置数据主题*/
#define BASEINFOR_TOPIC    "/bracelet/report/bracelet_info"   	/*手环基本数据主题*/
#define ATTENDANCE_TOPIC   "/bracelet/attendance/report"      	/*考勤数据主题*/
#define BLE_ATT_TOPIC      "/iot_gateway/ble_attendance/report" /*蓝牙无感考勤主题*/
#define SLEEP_TOPIC        "/bracelet/report/sleep_data"        /*睡眠数据*/

#define RESETACK_TOPIC     "/iot_gateway/reset/ack"             /*网关重置回执主题*/ 
#define SMSBCD_TOPIC       "/iot_gateway/notice/sms"          	/*消息回执主题*/
#define ADDRMANNABCD_TOPIC "/iot_gateway/management/addr"     	/*485地址管理回执主题*/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"          	/*升级主题回执主题*/
#define UPDATABLE_TOPIC    "/iot_gateway/braupdate/ack"         /*升级主题回执主题*/
#define BLEUPDATABCD_TOPIC "/iot_gateway/bleupdate/ack"         /*蓝牙升级主题回执主题*/
#define BAECON_TOPIC       "/iot_gateway/management/beacon"  	  /*信标管理回执*/
#define USERINFO_TOPIC     "/iot_gateway/user_info/ack"       	/*用户信息设置回执*/

#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"  	/*设备软件版本上报*/
#define ALARM_TOPIC        "/iot_gateway/device_alarm/report"   /*设备告警主题*/
#define SHT_TOPIC					 "/iot_gateway/temp_hum/report"  			/*温湿度数据上报*/

#define WILL_TOPIC         "/iot_gateway/offline_will/report"   /*遗嘱消息主题*/

/***************************到平台协议***************************/
#define SPORT              0x51        //运动数据Msg_id
#define HEALTH             0x52        //健康数据Msg_id
#define LOCATION           0x53        //位置数据Msg_id
#define BASEINFOR          0x54        //手环信息Msg_id
#define SLEEP_DATA         0x55        //睡眠数据Msg_id
#define SHT_DATA           0x56        //温湿度数据Msg_id
#define VERSION            0x57        //版本信息Msg_id
#define DEVRESTET          0xB0        //设备重置Msg_id

#define RESTETACK          0x0B        /*网关重置回执Msg_id*/
#define BANDDATABACK       0x1A        /*手环通信回执Msg_id*/
#define ADDRBACK           0x2A        /*蓝牙基站485地址回执Msg_id*/
#define BEACONBACK         0x4A        /*考勤信标回执Msg_id*/
#define BLEUPDATACK        0x8A        /*升级蓝牙回执Msg_id*/

#define MES_RX_BUF_SIZE	   10*1024
#define PLAT_MAX_DATA_LEN  1500

#define BLE_UPDATA_START   1
#define BRA_UPDATA_START   1
#define GAT_UPDATA_START   1
#define BLE_UPDATA_RESET   0
#define BRA_UPDATA_RESET   0
#define GAT_UPDATA_RESET   0

typedef struct
{
	msgTypes evt_id;
	uint8_t  socket;
	uint16_t payload_len;
	uint8_t  ble_updata_flag;
	uint8_t  bracelet_updata_flag;
	uint8_t  gateway_updata_flag;
	uint8_t  payload_data[PLAT_MAX_DATA_LEN];
}plat_report_t;

typedef enum 
{
	PLAT_RESPONSE_SEND_MESSAGE = 0, 
	PLAT_RESPONSE_ADDRMANAGEMENT,
	PLAT_RESPONSE_BEAMANAGEMENT,
	PLAT_RESPONSE_BRAUPDATE,
	PLAT_RESPONSE_DEVUPDATE,
	PLAT_RESPONSE_BLEUPDATE,
	PLAT_RESPONSE_USERINFOSET,
	PLAT_RESPONSE_DEVCFGSET,
	PLAT_RESPONSE_RESET
}app_plat_topic;

typedef struct
{
	uint8_t *pRxBuf;			         /* 接收缓冲区 */
	uint16_t usRxBufSize;		       /* 接收缓冲区大小 */
	
	volatile uint16_t usRxWrite;	 /* 接收缓冲区写指针 */
	volatile uint16_t usRxRead;		 /* 接收缓冲区读指针 */
	volatile uint16_t usRxCount;	 /* 还未读取的新数据个数 */
}app_plat_fifo;

extern char sms_topic[];
extern char base_topic[];
extern char userinfo_topic[];
extern char bleupdata_topic[];
extern char gatereset_topic[];
extern char gateupdata_topic[];
extern char devcfgpara_topic[];
//extern char beaconmanagement_topic[];
extern char bracelet_topic[];

void app_palt_fifoinit(void);
void app_plat_SendMessage(void);
void app_palt_SubscribandPublishMyTopic(void);

void app_palt_Connectack(plat_report_t * report_t);
void app_palt_Receicedata(plat_report_t * report_t);

void app_plat_SHTDataPublish(void);
void app_plat_HeartratePublish(uint8_t *bracelet,uint8_t heartrate);
void app_plat_SleepDataPublish(uint8_t *bracelet,uint8_t *sleepdata);
void app_plat_SportDataPublish(uint8_t *bracelet,uint8_t *sportdata);
void app_plat_HistorySportDataPublish(uint8_t *bracelet,uint8_t *sportdata);
void app_plat_BraceletInfoPublish(uint8_t *bracelet,uint8_t braceletelectricity);
void app_plat_AttendancePublish(uint8_t *bracelet,uint8_t *location,uint8_t *beastation,uint8_t inoutflag);
void app_plat_LocationPublish(uint8_t *bracelet,uint8_t *location,uint8_t beaconelectricity,uint8_t *beastation);

ErrorStatus app_palt_Reportparse(plat_report_t * report_t);
uint16_t app_plat_usMBCRC16( uint8_t * pucFrame, uint16_t usLen );

#endif
