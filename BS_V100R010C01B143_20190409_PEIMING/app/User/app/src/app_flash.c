#include "includes.h"

SysCfg sysCfg = {0};

//char default_server_user[] = "test";
//char default_server_pass[] = "123456";
//uint8_t default_server_ip[4]  = {139,159,213,146};

char default_server_user[] = "CdGateway";
char default_server_pass[] = "0d2ff5970d1c4759a6ade32da30866ab";
uint8_t default_server_ip[4]  = {139,159,133,76};

static uint32_t idAddr[]={0x1FFFF7AC,
												  0x1FFFF7E8, 
												  0x1FFF7A10,  
												  0x1FFFF7AC,  
												  0x1FFF7A10,  
												  0x1FF0F420,  
												  0x1FF80050,  
												  0x1FF80050,  
												  0x1FFF7590, 
												  0x1FF0F420}; 

/*
*********************************************************************************************************
 * ��������app_flash_GetSTM32MCUID
 * ����  ����ȡ�豸ID
 * ����  ��id�����id������
 *       ��type��MCU����
 * ����  : ��
*********************************************************************************************************
 */
static void app_flash_GetSTM32MCUID(uint32_t *id,MCUTypedef type)
{
	 if(id!=NULL)
	 {
		 id[0]=*(uint32_t*)(idAddr[type]);
		 id[1]=*(uint32_t*)(idAddr[type]+4);
		 id[2]=*(uint32_t*)(idAddr[type]+8);
	 } 
}

/*����ʱ*/
void delay_ms(u16 time)
{    
	 uint16_t i=0;  
	 while(time--)
	 {
			i=12000;  
			while(i--);    
	 }
}

/*
*********************************************************************************************************
 * ��������app_flash_ReadSysConfig
 * ����  ����������������Ϣ
 * ����  ����

 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR��ʧ��
*********************************************************************************************************
 */
ErrorStatus app_flash_LoadSysConfig(void)
{ 
	uint8_t tmp;
	uint8_t errcode;

	uint32_t addr[3] = {0};	
	uint8_t mac_id[12] = {0};
	uint8_t client_mac[6] = {0};
	
	char DeviceID[16]={0};
	
	#if APP_DEBUG
	printf("Load system config...%d\r\n",SYSCFG_DATA_LEN);
	#endif
	
	/*��ȡ���ò���*/
	errcode = bsp_ReadCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN);
	if(1 == errcode)
	{
		#if APP_DEBUG
		printf("ReadSysConfig:Read list fail.\r\n");
		#endif
		return ERROR;  
	} 

	app_flash_GetSTM32MCUID(addr,STM32F1);
	memcpy(mac_id,(uint8_t *)&addr,sizeof(mac_id));
		
	/*��ȡ�豸ID*/
	#if 1   
	memcpy(client_mac,mac_id,6);
	for(uint8_t i = 0; i < 6; i++ )
	{
		client_mac[i] ^= mac_id[6+i];
	}
	#else
	memcpy(client_mac,&mac_id[8],3);
	client_mac[3] = mac_id[5];
	
  tmp = mac_id[0];
	tmp <<= 4;
	tmp &= 0xf0;
	mac_id[2] &= 0x0f;
	tmp |= mac_id[2];
	client_mac[4] = tmp;

  tmp = mac_id[6];
	tmp <<= 4;
	tmp &= 0xf0;
	mac_id[7] &= 0x0f;
	tmp |= mac_id[7];
	client_mac[5] = tmp;	
	#endif
	
	client_mac[0] &= 0xfe;   /*��һ���ֽ�Ϊż��*/
	memcpy(sysCfg.parameter.client_mac,client_mac,6);
	
	sysCfg.parameter.register_flag = UNREGISTER;
	sysCfg.parameter.connect_state = DISCONNECT;
	
	/*�����ű����*/                          
	if(sysCfg.parameter.beacount > 25)
	{
    #if 0
		printf("ReadSysConfig:beacount over limit.\r\n");
		#endif
		sysCfg.parameter.beacount = 0;
	}

	if(  (0 == memcmp(sysCfg.parameter.server_ip,default_server_ip,sizeof(default_server_ip)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen(default_server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen((char *)sysCfg.parameter.server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen((char *)sysCfg.parameter.server_pass))))
	{
		sysCfg.parameter.config_hold_flag = 0xff;
	}
	
	/*û�б��޸Ĺ���ʹ��Ĭ�����ò���*/
	if(sysCfg.parameter.config_hold_flag != CFG_HOLDER)   
	{
		/*����Ĭ������*/
		memcpy(sysCfg.parameter.server_ip,default_server_ip,4);
		sysCfg.parameter.server_port = SERVER_PORT;		
		
		memcpy(sysCfg.parameter.server_user,default_server_user,strlen(default_server_user));
		memcpy(sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass));
		
		sysCfg.parameter.server_user[strlen(default_server_user)] = '\0';
		sysCfg.parameter.server_pass[strlen(default_server_pass)] = '\0';
			 
		sysCfg.parameter.data_socket = SOCK_TCPS;
		
		#if APP_DEBUG
		printf("Use default config.\r\n");
		#endif
	}
	else
	{
		#if APP_DEBUG
		printf("Use flash config.\r\n");
		#endif
		sysCfg.parameter.data_socket = SOCK_TCP;
	}

	sysCfg.parameter.client_port = CLIENT_PORT;
	
	sprintf(DeviceID,"%02x%02x%02x%02x%02x%02x%02x%02x",0,0,sysCfg.parameter.client_mac[0],sysCfg.parameter.client_mac[1],
	sysCfg.parameter.client_mac[2],sysCfg.parameter.client_mac[3],sysCfg.parameter.client_mac[4],sysCfg.parameter.client_mac[5]);
	
	memcpy(sysCfg.parameter.client_id,DeviceID,sizeof(DeviceID));	
	sysCfg.parameter.client_id[sizeof(DeviceID)] = '\0';
	
	/*������Ϣ�м����豸��Ϣ*/  	        
	memcpy(&sms_topic[7],DeviceID,sizeof(DeviceID));  	        
	memcpy(&userinfo_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&bleupdata_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&gateupdata_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&devcfgpara_topic[7],DeviceID,sizeof(DeviceID));
  //memcpy(&beaconmanagement_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&bracelet_topic[7],DeviceID,sizeof(DeviceID));
	memcpy(&gatereset_topic[7],DeviceID,sizeof(DeviceID));
	
	#if APP_DEBUG
	/*��ӡ���ò���*/
	printf("server IP:");
	for(uint8_t i=0;i<3;i++)
	{
		printf("%d",sysCfg.parameter.server_ip[i]);
		printf(".");
	}
	printf("%d",sysCfg.parameter.server_ip[3]);
	
	printf("      PORT:%d",sysCfg.parameter.server_port);
	printf("\r\n");
	
	printf("server username:%s\r\n",sysCfg.parameter.server_user);
	printf("server password:%s\r\n",sysCfg.parameter.server_pass);
	
	printf("Client 	MAC:");
	for(uint8_t i=0;i<6;i++)
	{
		printf("%02x ",sysCfg.parameter.client_mac[i]);
	}
	printf("\r\n");
	
	printf("Client id:%s\r\n",sysCfg.parameter.client_id);
	printf("Hardware version:%s\r\n",HARDWARE_VERSION);
	printf("Software version:%s\r\n",SOFTWARE_VERSION);
	
	printf("Ready to init net...\r\n");
	#endif

	return SUCCESS;
}

/*
*********************************************************************************************************
 * ��������app_flash_GetBeaconList
 * ����  ����ȡ�������ؿ����ű��б�
 * ����  ��Serialnumber,��Ϣ��ˮ��
 * ����  : ��
�����䷽�򡿣���������-->ƽ̨
��Э���ʽ����Len_l(1Byte)+Len_h(1Byte)+Msg_id(1Byte)+Dealtype(1Byte)+BeaconList(NByte)+Dealresult(1Byte)
							Serialnumber(4Byte)+CRC(2Byte)
*********************************************************************************************************
 */
void app_flash_GetBeaconList(uint8_t *Serialnumber)
{
	uint16_t crcdata,i;
	uint8_t bealistinfo[256];
	
	bealistinfo[0]=(sysCfg.parameter.beacount*4+11)%256;  /*���������ȣ��ߣ�*/
	bealistinfo[1]=(sysCfg.parameter.beacount*4+11)/256;  /*���������ȣ��ͣ�*/
	
	bealistinfo[2]=BEACONBACK;    /*��ϢID*/
  bealistinfo[3]=0x00;          /*��ȡ�б�*/

  memset(&bealistinfo[4],0,2);                           	/*�����豸ID���ֽڲ���*/
	memcpy(&bealistinfo[6],sysCfg.parameter.client_mac,6); 	/*����ID*/
	bealistinfo[12]=sysCfg.parameter.beacount;              /*�����ű����*/
  memcpy(&bealistinfo[13],(uint8_t *)sysCfg.parameter.bealist,sysCfg.parameter.beacount*4);
	memcpy(&bealistinfo[sysCfg.parameter.beacount*4+13],Serialnumber,4);            /*��ˮ��*/
	
	crcdata=app_plat_usMBCRC16(bealistinfo,bealistinfo[0]+bealistinfo[1]*256+2);    /*CRCУ��*/
	memcpy(&bealistinfo[bealistinfo[0]+bealistinfo[1]*256+2],(uint8_t *)&crcdata,2);
	if(!mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)bealistinfo , bealistinfo[0]+bealistinfo[1]*256+4))
	{
		return;
	}
	
	#if APP_DEBUG
	printf("beacount:%d Send beacon list data:\r\n",sysCfg.parameter.beacount);
	for(i=0;i<bealistinfo[0]+bealistinfo[1]*256+4;i++)
	{
		printf("%02x ",bealistinfo[i]);
	}
	printf("\r\n");
	#endif
}

/*
*********************************************************************************************************
 * ��������app_flash_AddBeacon
 * ����  �������µĿ����ű굽flash��
 * ����  ��
 *       ��beacon,��ӵĿ����ű�
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR��ʧ��
�����䷽�򡿣�ƽ̨-->��������
*********************************************************************************************************
 */
ErrorStatus app_flash_AddBeacon(uint8_t *beacon,uint8_t *Serialnumber)
{
	uint16_t crcdata;
	uint8_t i,j,err,failfalg=0;
	
	uint8_t checkdata[SYSCFG_DATA_LEN];
	
	uint8_t backinfo[23]={0x13,0x00,0x2a};
	
	backinfo[2]=BEACONBACK;
	backinfo[3]=0x01;                                    /*���������ű�*/
	
  memset(&backinfo[4],0,2);                            /*�����豸ID���ֽڲ���*/
	memcpy(&backinfo[6],sysCfg.parameter.client_mac,6);  /*����ID*/
	memcpy(&backinfo[13],beacon,4);                      /*Ҫ����ĵ�ַ*/
  memcpy(&backinfo[17],Serialnumber,4);                /*��ˮ��*/
	
	#if APP_DEBUG  /*����֮ǰ��MAC��ַ�б�*/
	printf("Add beacon:");
	for(i=0;i<4;i++)
	{
		printf("%02x ",beacon[i]);
	}
	printf("\r\n");
	printf("before add beacon count:%d list:\r\n",sysCfg.parameter.beacount);
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		for(j=0;j<4;j++)
		{
			printf("%02x ",sysCfg.parameter.bealist[i][j]);
		}
		printf("\r\n");
	}
	#endif
	
	/*����Ƿ��п����ű�*/
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		/*�����������иÿ����ű�*/
		if(0 == memcmp(beacon,sysCfg.parameter.bealist[i],4))
		{
			backinfo[12]=0x00;
			crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
			memcpy(&backinfo[21],(uint8_t *)&crcdata,2);
			
			if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
			{
				return SUCCESS;
			}
			else
			{
		  	#if APP_DEBUG
				printf("beacon already existing add beacon back data Send fail!\r\n");  
			  #endif
				return SUCCESS;
			}
		}
	}
	
	memcpy(&sysCfg.parameter.bealist[sysCfg.parameter.beacount],beacon,4);  
  sysCfg.parameter.beacount++;  
	sysCfg.parameter.config_hold_flag=CFG_HOLDER;
	
	#if APP_DEBUG  
	printf("after add beacon count:%d list:\r\n",sysCfg.parameter.beacount);
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		for(j=0;j<4;j++)
		{
			printf("%02x ",sysCfg.parameter.bealist[i][j]);
		}
		printf("\r\n");
	}
	#endif

	err=bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN);
	if(0!=err)      /*flashд�����*/
	{ 
		failfalg=1;
	}

	/*���д������*/
	err=bsp_ReadCpuFlash(SYSCFG_ADDR,checkdata,SYSCFG_DATA_LEN);
	if(1==err)
	{
		failfalg=1;
		return ERROR;   /*��ȡʧ��*/
	}
	
	/*�Ƚ�д����������������Ƿ�һ��*/
	if(0 != memcmp(checkdata,sysCfg.data,SYSCFG_DATA_LEN))
	{
		failfalg=1;
	}

	if(1==failfalg)
	{
		/*д�����ݴ���*/
		backinfo[12]=0x01;           
		crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
		memcpy(&backinfo[21],(uint8_t *)&crcdata,2);

		if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
		{
			#if APP_DEBUG
			printf("add beacon write flash error back data:\r\n");
		  #endif
			return SUCCESS;
		}
		else
		{
		  #if APP_DEBUG
			printf("add beacon write flash error back data send fail!\r\n");  
		  #endif
		}
		return ERROR;
	}

	backinfo[12]=0x02;           
	crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
	memcpy(&backinfo[21],(uint8_t *)&crcdata,2);
	
	if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
	{
		return SUCCESS;
	}
	else
	{
		#if APP_DEBUG	
	  printf("add beacon write flash success back data send fail!\r\n"); 
		#endif		
	}	 
	return SUCCESS;
}

/*
*********************************************************************************************************
 * ��������app_flash_DeteStation
 * ����  ��ɾ�������ű�
 * ����  ��
 *       ��beacon,ɾ����beacon
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR��ʧ��
�����䷽�򡿣���������-->ƽ̨
��Э���ʽ����Len_l(1Byte)+Len_h(1Byte)+Msg_id(1Byte)+Dealtype(1Byte)+DeviceID(8Byte)+Dealresult(1Byte)
							NrfID(8Byte)+Serialnumber(4Byte)+CRC(2Byte)
*********************************************************************************************************
 */
ErrorStatus app_flash_DeleBeacon(uint8_t *beacon,uint8_t *Serialnumber)
{
	uint8_t i,j,err;
	uint16_t crcdata;
	uint8_t failfalg=0;

	uint8_t table[8]={0};
	uint8_t checkdata[SYSCFG_DATA_LEN];
	uint8_t backinfo[23]={0x13,0x00,0x2a};

	backinfo[2]=BEACONBACK;
	backinfo[3]=0x02;               /*ɾ��һ�������ű�*/

  memset(&backinfo[4],0,2);                            /*�����豸ID���ֽڲ���*/
	memcpy(&backinfo[6],sysCfg.parameter.client_mac,6);  /*����ID*/
	memcpy(&backinfo[13],beacon,4);                      /*Ҫɾ���ĵ�ַ*/
  memcpy(&backinfo[17],Serialnumber,4);      					 /*��ˮ��*/
	
	#if APP_DEBUG  /*ɾ��֮ǰ��MAC��ַ�б�*/
	printf("Dele bea:");
	for(i=0;i<4;i++)
	{
		printf("%02x ",beacon[i]);
	}
	printf("\r\nbefore delete beacon count:%d list:\r\n",sysCfg.parameter.beacount);
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		for(j=0;j<4;j++)
		{
			printf("%02x ",sysCfg.parameter.bealist[i][j]);
		}
		printf("\r\n");
	}
	#endif
	
	/*����Ƿ��и�������վ*/
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		/*�������������溬�и�������վ ��������*/
		if(0 == memcmp(beacon,sysCfg.parameter.bealist[i],4))
		{
		  #if APP_DEBUG
			printf("find delete beacon.\r\n");
			#endif
			break;
		}
	}
		
	/*û��Ҫɾ����������վ*/
	if(i==sysCfg.parameter.beacount)  
	{
		backinfo[12]=0x00;
		crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
		memcpy(&backinfo[21],(uint8_t *)&crcdata,2);
		
		if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
		{
		}
		else
		{
		  #if APP_DEBUG
			printf("not have dele beacon back data Send fail!\r\n");  
		  #endif
		}
		return ERROR;
	}
  
	for(j=0;j<(sysCfg.parameter.beacount-i-1);j++)   /*����beacon�б�*/
	{
		memcpy(table,sysCfg.parameter.bealist[i+j+1],4);
		memcpy(sysCfg.parameter.bealist[i+j],table,4);
	}
	
	sysCfg.parameter.beacount--;   /*������1*/
	sysCfg.parameter.config_hold_flag=CFG_HOLDER;
	
	#if APP_DEBUG   /*ɾ�����MAC��ַ�б�*/
	printf("after delete beacon count:%d list:\r\n",sysCfg.parameter.beacount);
	for(i=0;i<sysCfg.parameter.beacount;i++)
	{
		for(j=0;j<4;j++)
		{
			printf("%02x ",sysCfg.parameter.bealist[i][j]);
		}
		printf("\r\n");
	}
	#endif

	err=bsp_WriteCpuFlash(SYSCFG_ADDR,sysCfg.data,SYSCFG_DATA_LEN);
	if(0!=err)      /*flashд�����*/
	{ 
		failfalg=1;	
	}

	/*���д������*/
	err=bsp_ReadCpuFlash(SYSCFG_ADDR,checkdata,SYSCFG_DATA_LEN);
	if(1==err)
	{
		failfalg=1;
		return ERROR;   /*��ȡʧ��*/
	}

	 if(0 != memcmp(checkdata,sysCfg.data,SYSCFG_DATA_LEN))
	 {
		 failfalg=1;
	}

	if(1==failfalg)
	{
		/*д�����ݴ���*/
		backinfo[12]=0x01;           
		crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
		memcpy(&backinfo[21],(uint8_t *)&crcdata,2);
		
		if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
		{
		  #if APP_DEBUG
			printf("delete beacon write flash error back data:\r\n");
			for(i=0;i<backinfo[0]+backinfo[1]*256+4;i++)
			{
				printf("%02x ",backinfo[i]);
			}
			printf("\r\n");
			#endif
			return SUCCESS;
		}
		else
		{
		  #if APP_DEBUG
			printf("delete beacon write flash error back data send fail!\r\n");
      #endif			
		}
		return ERROR;
	}

	backinfo[12]=0x02;
	crcdata=app_plat_usMBCRC16(backinfo,backinfo[0]+backinfo[1]*256+2);
	memcpy(&backinfo[21],(uint8_t *)&crcdata,2);

	if(mqtt_publish( SOCK_TCPS , BAECON_TOPIC , (char *)backinfo , backinfo[0]+backinfo[1]*256+4))
	{
	} 
	else
	{
    #if APP_DEBUG
		printf("delete beacon success back data Send fail!\r\n");  
    #endif
	}
	 
	return SUCCESS;
}

