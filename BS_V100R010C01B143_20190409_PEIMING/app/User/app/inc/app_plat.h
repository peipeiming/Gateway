#ifndef __APP_PLAT_H
#define __APP_PLAT_H

#include "stm32f10x.h"
#include "MQTTPacket.h"

/***************************��������*****************************/
#define HIS_SPORT_TOPIC    "/bracelet/report/history_sport_data"/*��ʷ�˶���������*/
#define SPORT_TOPIC        "/bracelet/report/sport_data"     	  /*�˶���������*/
#define HEALTH_TOPIC       "/bracelet/report/heartrate_data" 	  /*������������*/
#define LOCATION_TOPIC     "/bracelet/report/location"        	/*λ����������*/
#define BASEINFOR_TOPIC    "/bracelet/report/bracelet_info"   	/*�ֻ�������������*/
#define ATTENDANCE_TOPIC   "/bracelet/attendance/report"      	/*������������*/
#define BLE_ATT_TOPIC      "/iot_gateway/ble_attendance/report" /*�����޸п�������*/
#define SLEEP_TOPIC        "/bracelet/report/sleep_data"        /*˯������*/

#define RESETACK_TOPIC     "/iot_gateway/reset/ack"             /*�������û�ִ����*/ 
#define SMSBCD_TOPIC       "/iot_gateway/notice/sms"          	/*��Ϣ��ִ����*/
#define ADDRMANNABCD_TOPIC "/iot_gateway/management/addr"     	/*485��ַ�����ִ����*/
#define UPDATABCD_TOPIC    "/iot_gateway/update/ack"          	/*���������ִ����*/
#define UPDATABLE_TOPIC    "/iot_gateway/braupdate/ack"         /*���������ִ����*/
#define BLEUPDATABCD_TOPIC "/iot_gateway/bleupdate/ack"         /*�������������ִ����*/
#define BAECON_TOPIC       "/iot_gateway/management/beacon"  	  /*�ű�����ִ*/
#define USERINFO_TOPIC     "/iot_gateway/user_info/ack"       	/*�û���Ϣ���û�ִ*/

#define SOFTVERSION_TOPIC  "/json/iot_gateway/version/report"  	/*�豸����汾�ϱ�*/
#define ALARM_TOPIC        "/iot_gateway/device_alarm/report"   /*�豸�澯����*/
#define SHT_TOPIC					 "/iot_gateway/temp_hum/report"  			/*��ʪ�������ϱ�*/

#define WILL_TOPIC         "/iot_gateway/offline_will/report"   /*������Ϣ����*/

/***************************��ƽ̨Э��***************************/
#define SPORT              0x51        //�˶�����Msg_id
#define HEALTH             0x52        //��������Msg_id
#define LOCATION           0x53        //λ������Msg_id
#define BASEINFOR          0x54        //�ֻ���ϢMsg_id
#define SLEEP_DATA         0x55        //˯������Msg_id
#define SHT_DATA           0x56        //��ʪ������Msg_id
#define VERSION            0x57        //�汾��ϢMsg_id
#define DEVRESTET          0xB0        //�豸����Msg_id

#define RESTETACK          0x0B        /*�������û�ִMsg_id*/
#define BANDDATABACK       0x1A        /*�ֻ�ͨ�Ż�ִMsg_id*/
#define ADDRBACK           0x2A        /*������վ485��ַ��ִMsg_id*/
#define BEACONBACK         0x4A        /*�����ű��ִMsg_id*/
#define BLEUPDATACK        0x8A        /*����������ִMsg_id*/

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
	uint8_t *pRxBuf;			         /* ���ջ����� */
	uint16_t usRxBufSize;		       /* ���ջ�������С */
	
	volatile uint16_t usRxWrite;	 /* ���ջ�����дָ�� */
	volatile uint16_t usRxRead;		 /* ���ջ�������ָ�� */
	volatile uint16_t usRxCount;	 /* ��δ��ȡ�������ݸ��� */
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
