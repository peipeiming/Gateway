#ifndef __APP_FLASH_H
#define __APP_FLASH_H

#include "stm32f10x.h"

/*****************************************
系统参数存储地址 预留空间大小 512 Byte
存储顺序（从低到高字节）：
0~30Byte:网络连接参数
36~ :地址参数
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

/*配置信息结构体 字对齐*/
typedef struct
{
	/*连接参数*/   
	uint8_t  server_ip[4];  
	uint8_t  server_user[20];
	uint8_t  server_pass[50];    
	
	uint16_t server_port;  
	uint8_t  config_hold_flag;   
	
	uint8_t  ip[4];							/*local IP本地IP地址*/
  uint8_t  sub[4];						/*子网掩码*/
  uint8_t  gw[4];							/*网关*/	
	uint8_t  dhcp;              /*DHCP 标志位*/	

	uint8_t  beacount;          /*蓝牙信标个数*/
	uint8_t  bealist[25][4];    /*蓝牙考勤信标组数*/ 
	
	/*不需要保存的参数*/
	uint8_t  connect_state;
  uint8_t  data_socket;       /*数据通道*/
	
  uint16_t client_port;
  uint8_t  client_ip[4];  
	uint8_t  client_id[18];
	uint8_t  client_mac[6];	
	
	uint8_t  register_flag;
	uint16_t key_time_count;
	uint8_t  nrfstation[8];   /*485地址信息*/
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

