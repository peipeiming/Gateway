#ifndef _BSP_MQTT_OTA_H_
#define _BSP_MQTT_OTA_H_

#include "stm32f10x.h"

/********************FLASH����*****************************
Bootloder:   0x08000000--0x08007FFF        32K
Application: 0x08008000--0x080437FF        238K
Backup:      0x08043800--0x0807EFFF        238k
Flag:        0x0807F000--0x0807FFFF        4K
**********************************************************/

#define FIRM_INFO_LEN       8             /*�̼�ͷ����Ч��Ϣ�ֽ���*/

/*�豸����*/
#define GATEWAY             0x01          /*�����豸*/
#define RFID                0x02					/*ˢ�����豸*/

/*��������*/
#define STM32F103ZET6       0x01   
#define STM32F103RCT6       0x02
#define STM32F030C8T6       0x03  

/*��������*/
#define TRANS_WW5500        0x01
#define TRANS_GSM           0x02

/*SRAM ����*/                 
#define NOSRAM              0x01

#define SRAM                0x02

/*�����汾��*/
#define SOFTVERSION         14

#define UPDATA     			 		0x3A          /*������ӦMsg_id*/
#define BLE_UPDATA     			0x9A          /*������ӦMsg_id*/
#define ACK_FAIL     			 	0x00          /*����ʧ��*/
#define ACK_OK     			 		0x01          /*�����ɹ�*/
#define OTA_STA    			 		0x02          
#define KEY_OK    			 		0x03          

#define ACK_ERASE_ERROR  		0x02          /*����Flashʧ��*/
#define ACK_WRITE_ERROR  		0x03          /*дFlashʧ��*/
#define ACK_LEN_ERROR    		0x04					/*�̼������쳣*/
#define ACK_FLAG_ERROR   		0x05					/*д��������־λʧ��*/
#define ACK_CRC16_ERROR 		0x06					/*CRCУ��ʧ��*/
#define ACK_ADDROV_ERROR 	  0x07          /*д��ַԽ��*/

#define ACK_FIRTYPE_ERROR   0x08          /*�̼����ʹ���*/	
#define ACK_DEVTYPE_ERROR   0x09          /*�豸���ʹ���*/
#define ACK_MCUTYPE_ERROR   0x0A          /*�������ʹ���*/
#define ACK_TRSTYPE_ERROR   0x0B          /*�������ʹ���*/
#define ACK_SRATYPE_ERROR   0x0C          /*SRAM���ʹ���*/
#define ACK_VERTYPE_ERROR   0x0D   				/*�����汾����*/

#define APPADRR        		  0x08008000
#define BACKUPADRR 			 		0x08043800    /*���򱸷�����ַ*/
#define FLAGADRR   			 		0x0807F000    /*��־λ����ַ*/

void bsp_ota_UpdataGateway(uint16_t messgelen,uint8_t* payload);
void bsp_ota_UpdataBracelet(uint16_t messgelen,uint8_t* payload);
void bsp_ble_ackResponse( uint8_t asktype , uint8_t *serialnumber, uint8_t *bracelet_mac);

#endif

