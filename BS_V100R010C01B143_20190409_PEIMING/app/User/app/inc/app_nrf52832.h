#ifndef __APP_NRF52832_H
#define __APP_NRF52832_H

#include "stm32f10x.h"		

#define HIS_SPORT_PUBLISH    0x01
#define HIS_SPORT_CLEAR      0x00

#define SOS_ON               0x01
#define SOS_OFF              0x02

#define IN_AREA      	   		 0x01
#define OUT_AREA         		 0x00

#define NRF_PORT             COM3	       //控制器与nrf通信端口
#define BRACELET_RECORD_LEN  24          //每条需要保存的手环数据的长度
#define BRACELET_COUNT       300
#define MAX_BLE_RESPONSE_LEN 50

/*********************************nrf协议命令********************************************/
#define DISCONNECTBLE                   0x40        //断开BLE连接Msg_id
#define GETBLEDATA                      0x41        //获取手环数据Msg_id
#define CONNECTBLE                      0x42        //连接手环Msg_id
#define SEND_MESSAGE                    0x43        //留言Msg_id
#define GET_STATION                     0x44        //获取基站
#define RESET                           0x45        //复位基站
#define UPDATA_STATION                  0x46        //升级蓝牙
#define SET_TIME                        0x47        //设置RTC时间

#define OTA_CMD                         0x49        //手环OTA

void app_nrf_DealBleData(void);
void app_nrf_ResetStation(void);
void app_nrf_GetNewStation(void);
void app_nrf_SetRssi(uint8_t rssi);
void app_nrf_UpdataBle(uint8_t *station);
void app_nrf_GetBleData(uint8_t *station);
ErrorStatus app_nrf_ReadData(uint8_t *recebuf);
ErrorStatus app_nrf_DisconnectBle(uint8_t *station);
ErrorStatus app_nrf_ConnectBle(uint8_t *station,uint8_t *bracelet);
ErrorStatus app_nrf_LeaveMessage(uint8_t *station,uint8_t *message,uint16_t len);

#endif

