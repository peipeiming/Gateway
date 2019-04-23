#ifndef __APP_FLASH_H
#define __APP_FLASH_H

#include "stm32f10x.h"

/*****************************************
ϵͳ�����洢��ַ Ԥ���ռ��С 512 Byte
�洢˳�򣨴ӵ͵����ֽڣ���
0~30Byte:�������Ӳ���
36~ :��ַ����
*****************************************/
#define SYSCFG_ADDR        (0x08000000 + 0x80000 - 0x200)

#define REGISTER           0x01
#define UNREGISTER         0x00


#define CFG_HOLDER	       0xCF

#define CONNECT    				 0x01
#define DISCONNECT   			 0x00

#define SERVER_PORT        1883
#define CLIENT_PORT        1000
#ifdef  MCBR03
#define HARDWARE_VERSION   "MCBR03_V1.2"  
#else
#define HARDWARE_VERSION   "MCBR02_V1.7"  
#endif

#define SOFTWARE_VERSION   "BS_V100R010C01B143_0409" 

#define MAX_STATION_COUNT  20
#define SYSCFG_DATA_LEN    90

typedef enum {
		STM32F0, 
		STM32F1,
		STM32F2,
		STM32F3,
		STM32F4,
		STM32F7,
		STM32L0,
		STM32L1,
		STM32L4,
		STM32H7,
 }MCUTypedef;

/*������Ϣ�ṹ�� �ֶ���*/
typedef struct
{
	/*���Ӳ���*/   
	uint8_t  server_ip[4];  
	uint8_t  server_user[20];
	uint8_t  server_pass[50];    
	
	uint16_t server_port;  
	uint8_t  config_hold_flag;   
	
	uint8_t  ip[4];							/*local IP����IP��ַ*/
  uint8_t  sub[4];						/*��������*/
  uint8_t  gw[4];							/*����*/	
	uint8_t  dhcp;              /*DHCP ��־λ*/	

	uint8_t  beacount;          /*�����ű����*/
	uint8_t  bealist[25][4];    /*���������ű�����*/ 
	
	/*����Ҫ����Ĳ���*/
	uint8_t  connect_state;
  uint8_t  data_socket;       /*����ͨ��*/
	
  uint16_t client_port;
  uint8_t  client_ip[4];  
	uint8_t  client_id[18];
	uint8_t  client_mac[6];	
	
	uint8_t  register_flag;
	uint16_t key_time_count;
	uint8_t  nrfstation[8];   /*485��ַ��Ϣ*/
}CfgPara;

typedef union
{
	CfgPara parameter;
	uint8_t data[SYSCFG_DATA_LEN];
}SysCfg;

extern SysCfg sysCfg;
extern char default_server_user[];
extern char default_server_pass[];
extern uint8_t default_server_ip[4];

void delay_ms(u16 time);
//void app_flash_GetBeaconList(uint8_t *Serialnumber);

ErrorStatus app_flash_LoadSysConfig(void);

//ErrorStatus app_flash_AddBeacon(uint8_t *beacon,uint8_t *Serialnumber);
//ErrorStatus app_flash_DeleBeacon(uint8_t *beacon,uint8_t *Serialnumber);

#endif

