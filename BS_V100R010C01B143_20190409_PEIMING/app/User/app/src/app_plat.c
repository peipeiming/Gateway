#include "includes.h"

static app_plat_fifo mes_fifo;
static uint8_t mes_rxbuf[MES_RX_BUF_SIZE];

/****************************��������******************************/
char sms_topic[]="/devid_0000000000000000/notice/sms";
char bleupdata_topic[]="/devid_0000000000000000/update/ble";
char userinfo_topic[]="/devid_0000000000000000/user_info/set";
char devcfgpara_topic[]="/devid_0000000000000000/devcfgpara/set";
char gatereset_topic[]="/devid_0000000000000000/reset/gateway";
char gateupdata_topic[]="/devid_0000000000000000/update/gateway";
//char beaconmanagement_topic[]="/devid_0000000000000000/management/beacon";
char bracelet_topic[]="/devid_0000000000000000/update/bracelet";

/*
*********************************************************************************************************
 * ��������app_palt_WriteFifoData
 * ����  ����Ϣ���ݴ��뻺����
 * ����  ��p�����洢������
 *       : len�����洢���ݳ���
 * ����  : ��
*********************************************************************************************************
 */
static void app_palt_WriteFifoData(uint8_t *p,uint8_t len)
{
	uint8_t i;
		
	mes_fifo.pRxBuf[mes_fifo.usRxWrite]=0x5a;
	if (++mes_fifo.usRxWrite >= mes_fifo.usRxBufSize)
	{
		mes_fifo.usRxWrite = 0;
	}
	
	mes_fifo.usRxCount++;
	for(i=0;i<len;i++)
	{
		/*�������ȡ����������С��FIFO�Ĵ�С,��д��*/
		if (mes_fifo.usRxCount<mes_fifo.usRxBufSize)
		{
			mes_fifo.pRxBuf[mes_fifo.usRxWrite]=p[i];
			if (++mes_fifo.usRxWrite >= mes_fifo.usRxBufSize)
			{
				mes_fifo.usRxWrite = 0;
			}
			mes_fifo.usRxCount++;
		}
	}
}

/*
*********************************************************************************************************
 * ��������app_palt_UpdataBle
 * ����  ����ʼ��������
 * ����  ��TopicInfo,������Ϣ
 * ����  : ��
*********************************************************************************************************
 */
static void app_palt_UpdataBle(uint8_t *TopicInfo)
{
	int32_t ret;
	uint16_t crcdata;
	uint16_t port = 0;
	USART_InitTypeDef USART_InitStructure;
	
	uint8_t ble_ip[4] = {0};
	uint8_t topicdata[18]={0x0e,0x00,0x8a};
	static uint32_t UpdataBleSerialNumber=0;
	
	memcpy((uint8_t *)&port,&TopicInfo[15],2);  /*��ȡ�˿ں�*/
	
	app_nrf_UpdataBle(&TopicInfo[3]);           /*���Ϳ�ʼ��������*/
			 
	topicdata[2] = BLEUPDATACK;      
  memset(&topicdata[3],0x00,2);	
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);
	memcpy(&topicdata[12],(uint8_t *)&UpdataBleSerialNumber,4); /*��ˮ��*/
	UpdataBleSerialNumber++;
		
	memcpy(ble_ip,&TopicInfo[11],4);
	memcpy((uint8_t *)&port,&TopicInfo[15],2);
	
	#if APP_DEBUG
	printf("Updata ble device:");
	for(uint8_t i = 0; i < 8; i++)
	{
		printf("%02x ",TopicInfo[10-i]);
	}
	printf("\r\n");
	
	printf("Updata ble server IP:");
	for(uint8_t i = 0; i < 4; i++)
	{
		printf("%d ",TopicInfo[11+i]);
	}
	printf("      PORT:%d\r\n",port);
	#endif
		
	topicdata[11] = 0x01;
	
	/*�½�һ��Socket���󶨱��ض˿�5000*/
	ret = socket(SOCK_BLE,Sn_MR_TCP,5000,0x00);
	if(ret != SOCK_BLE)
	{
		topicdata[11] = 0x00;
		#if APP_DEBUG
		printf("%d:Socket Error\r\n",SOCK_BLE);
		#endif
	}

	/*����TCP������*/
	ret = connect(SOCK_BLE,ble_ip,port); 
	if(ret != SOCK_OK)
	{
		topicdata[11] = 0x00;
		#if APP_DEBUG
		printf("%d:Socket Connect Error\r\n",SOCK_BLE);
		#endif
	}
		
	USART_Cmd(USART3, DISABLE);		
	
	USART_InitStructure.USART_BaudRate = 9600;	/* ������ */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_Cmd(USART3, ENABLE);		/* ʹ�ܴ��� */

	vTaskDelay(50);
	
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2);  
	
  if(mqtt_publish( SOCK_TCPS, BLEUPDATABCD_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
	
	vTaskDelay(500);
	mqtt_disconnect();
}


/*
*********************************************************************************************************
 * ��������app_palt_BandData
 * ����  ���ֻ�ͨ�ţ��·�SMS��Ϣ
 * ����  ��TopicInfo,������Ϣ
 * 	     ��Len��������Ϣ�ܳ���
 * ����  : ��
*********************************************************************************************************
 */
static void app_palt_BandData(uint8_t *TopicInfo,uint16_t  Len)
{
  uint16_t crcdata;
	uint8_t backdata[26]={0x16,0x00,0x1A};
	
	backdata[2] = BANDDATABACK;
	memcpy(&backdata[3],&TopicInfo[11],8);         /*�ֻ�MAC��С�ˣ�*/
	memcpy(&backdata[11],&TopicInfo[19],8);        /*��Ϣ����ʱ���*/
	memcpy(&backdata[20],&TopicInfo[Len-6],4);     /*��ˮ��*/
		
  /*�������� �������������ͷ��ID*/
	if(SUCCESS != app_nrf_ConnectBle(&TopicInfo[3],&TopicInfo[11]))
	{
		#if APP_DEBUG
		printf("app_palt_BandData:Link bracelet fail.\r\n");
		#endif
		backdata[19]=0x01;    
		crcdata=app_plat_usMBCRC16(backdata,24);       /*CRCУ��*/
		memcpy(&backdata[24],(uint8_t *)&crcdata,2);		
    mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);

		if(ERROR == app_nrf_DisconnectBle(&TopicInfo[3]))
		{
			#if APP_DEBUG
			printf("app_palt_BandData:disconnet error.\r\n");
			#endif			
		}
    	
    ErrorLog(BRACELET,&TopicInfo[11],WARN,MESEG_WARN,0,NULL); /*�豸�澯*/
		return;    	        /*����ʧ��*/	
	} 
	
	vTaskDelay(1000);	
	if(SUCCESS == app_nrf_LeaveMessage(&TopicInfo[3],&TopicInfo[27],Len-33)) 
	{
		#if APP_DEBUG
		printf("Leave Message OK.\r\n");
		#endif
		backdata[19]=0x03;  /*�ֻ��յ�*/
		crcdata=app_plat_usMBCRC16(backdata,24);       /*CRCУ��*/
		memcpy(&backdata[24],(uint8_t *)&crcdata,2);	
		mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);

		vTaskDelay(1000);
		return;
	}  
	
	backdata[19]=0x02; 												  /*�ֻ�δ�յ���Ϣ*/
	crcdata=app_plat_usMBCRC16(backdata,24);    /*CRCУ��*/
  memcpy(&backdata[24],(uint8_t *)&crcdata,2);	
	mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);
  
	#if APP_DEBUG
	printf("app_palt_BandData:Leave a message fail.\r\n");
	#endif
	if(ERROR == app_nrf_DisconnectBle(&TopicInfo[3]))
	{
	}	
	
	ErrorLog(BRACELET,&TopicInfo[11],WARN,MESEG_WARN,0,NULL);  /*�豸�澯*/
	vTaskDelay(1000);
}

/*
*********************************************************************************************************
 * ��������app_plat_GetRespone
 * ����  ����ȡ��Ϣ����
 * ����  ��topic,��Ϣ����
 *			 : pRespone�������Ϣ����
 * ����  : 
 *       ��SUCCESS,�Ǵ��������Ϣ���ͣ�
 *       ��ERROR�����Ǵ��������Ϣ����
*********************************************************************************************************
*/
static ErrorStatus app_plat_GetRespone(uint8_t *TopicInfo,char *topic,app_plat_topic* pRespone)
{ 
	#if 0
	printf("Receive topic:%s\r\n",topic);
	#endif
	
   /*�·���Ϣ����*/
	if( NULL!=strstr( topic, "/notice/sms"))
	{			
    *pRespone = PLAT_RESPONSE_SEND_MESSAGE;
		return SUCCESS;
	}

	/*�����ű��·�*/
	else if( strstr( topic, "/management/beacon"))
	{
		*pRespone = PLAT_RESPONSE_BEAMANAGEMENT;
		return SUCCESS;
	}
	
	/*�����豸����*/
	else if( strstr( topic, "/update/ble"))
	{
		*pRespone = PLAT_RESPONSE_BLEUPDATE;
		return SUCCESS;
	}

	/*�����豸����*/
	else if( strstr( topic, "/update/gateway"))
	{
		*pRespone = PLAT_RESPONSE_DEVUPDATE;
		return SUCCESS;
	}
		/*�����ֻ�����*/
	else if( strstr( topic, "/update/bracelet"))
	{
		*pRespone = PLAT_RESPONSE_BRAUPDATE;
		return SUCCESS;
	}
	
	/*�û���Ϣ*/
	else if( strstr( topic, "/user_info/set"))
	{
		*pRespone = PLAT_RESPONSE_USERINFOSET;
		return SUCCESS;
	}

	/*�����豸��������*/
	else if( strstr( topic, "/devcfgpara/set"))
	{
		*pRespone = PLAT_RESPONSE_DEVCFGSET;
		return SUCCESS;
	}

	/*��������Ӳ����λ*/
	else if( strstr( topic, "/reset/gateway"))
	{
		*pRespone = PLAT_RESPONSE_RESET;
		return SUCCESS;
	}
	
	else
	{
		return ERROR;
	}
}

/*
*********************************************************************************************************
 * ��������app_palt_BeaconManagement
 * ����  �������ű����
 * ����  ��
 *       : TopicInfo,������Ϣ
 * ����  : ��
*********************************************************************************************************
*/
//static void app_palt_BeaconManagement(uint8_t *TopicInfo)
//{
//	/*��һ���ű������ڲ࣬�ڶ����ű��������*/
//	switch(TopicInfo[3])
//	{
//		/*��ȡ�ű��б�*/
//		case 0x00:
//			app_flash_GetBeaconList(&TopicInfo[12]);
//			break;
//		
//		/*����һ���ű�*/
//		case 0x01:
//			app_flash_AddBeacon(&TopicInfo[4],&TopicInfo[8]);
//			break;
//		
//		/*ɾ��һ���ű�*/
//		case 0x02:
//			app_flash_DeleBeacon(&TopicInfo[4],&TopicInfo[8]);
//			break;
//		
//		default:
//			break;
//	}
//}

/*
*********************************************************************************************************
 * ��������app_palt_userinfoset
 * ����  ���û���Ϣ����
 * ����  ��
 *       : TopicInfo,������Ϣ
 *       : len,������Ϣ
 * ����  : ��
*********************************************************************************************************
*/
static void app_palt_userinfoset(uint8_t *TopicInfo , uint16_t len )
{
	uint16_t crcdata;
	uint16_t userlen;
  uint8_t userinfo[512]={0};
	uint8_t ask[26]={0x16,0x00,0x6a};
	
	if(len > 512)
	{
		return;
	}
	
	userlen=len-33+1;                        									/*�û���Ϣ����*/
	memcpy( userinfo , TopicInfo , 27 );
	memcpy( &userinfo[27] , (uint8_t *)&userlen , 2 );
	userinfo[29]=1;                        						  			/*��־λ*/
	memcpy( &userinfo[30] , &TopicInfo[27] , len-33 );
	crcdata=app_plat_usMBCRC16( &userinfo[27] , len-33+3 );
	memcpy( &userinfo[30+len-33], (uint8_t *)&crcdata , 2 );  
	memcpy( &userinfo[32+userlen], &TopicInfo[len-6] , 4 );   /*ԭ��Ϣ�е���ˮ��*/
  userlen=TopicInfo[0]+TopicInfo[1]*256+5;
	memcpy( userinfo , (uint8_t *)&userlen , 2 );
	
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(userinfo,userinfo[1]*256+userinfo[0]+2);
	memcpy(&userinfo[userinfo[1]*256+userinfo[0]+2],(uint8_t *)&crcdata,2);  
	
  #if 0
	printf("topic data:");
	for(i=0;i<len;i++)
	{
		printf("%02x ",TopicInfo[i]);
	}
	printf("\r\n");
	printf("after deal:");
	for(i=0;i<userinfo[1]*256+userinfo[0]+4;i++)
	{
		printf("%02x ",userinfo[i]);
	}
	printf("\r\n");
	#endif
	
	memcpy(&ask[3],&TopicInfo[11],8);               /*�ֻ�MAC*/
	memcpy(&ask[11],&TopicInfo[19],8);
	ask[19]=0x03;        														
	/*�ֻ��յ�*/
	memcpy(&ask[20],&TopicInfo[len-6],4);
		
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(ask,ask[1]*256+ask[0]+2);
	memcpy(&ask[24],(uint8_t *)&crcdata,2);  
	
	/*��������*/
  mqtt_publish( sysCfg.parameter.data_socket , USERINFO_TOPIC , (char *)ask , ask[1]*256+ask[0]+4);
	
	app_palt_WriteFifoData(userinfo,userinfo[1]*256+userinfo[0]+4);
}

/*
*********************************************************************************************************
 * ��������app_palt_devcfgset
 * ����  �����ӵڵ�������������Ϣ����
 * ����  ��
 *       : TopicInfo,������Ϣ
 *       : len,������Ϣ
 * ����  : ��
*********************************************************************************************************
*/
static void app_palt_devcfgset(uint8_t *TopicInfo , uint16_t len )
{
	uint16_t usTemp;
	uint32_t _ulFlashAddr;
	
	uint8_t err_code = 0;
	
	if(TopicInfo[9] > sizeof(sysCfg.parameter.server_user) || TopicInfo[10+TopicInfo[9]] > sizeof(sysCfg.parameter.server_pass))
	{
		#if APP_DEBUG   
		printf("\r\nset parameter over limit!\r\n");
		#endif
		return;			
	}
		
	memcpy(sysCfg.parameter.server_ip,&TopicInfo[3],4);
	memcpy((uint8_t *)&sysCfg.parameter.server_port,&TopicInfo[7],2);
	
	memcpy(sysCfg.parameter.server_user,&TopicInfo[10],TopicInfo[9]);
	memcpy(sysCfg.parameter.server_pass,&TopicInfo[11+TopicInfo[9]],TopicInfo[10+TopicInfo[9]]);

	sysCfg.parameter.server_user[TopicInfo[9]] = '\0';
	sysCfg.parameter.server_pass[TopicInfo[10+TopicInfo[9]]] = '\0';
	
	if(  (0 == memcmp(sysCfg.parameter.server_ip,default_server_ip,sizeof(default_server_ip)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen(default_server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_user,default_server_user,strlen((char *)sysCfg.parameter.server_user))) 
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen(default_server_pass)))
		&& (0 == strncmp((char *)sysCfg.parameter.server_pass,default_server_pass,strlen((char *)sysCfg.parameter.server_pass))))
	{
		sysCfg.parameter.config_hold_flag = 0xff;
	}
	else
	{
		sysCfg.parameter.config_hold_flag = CFG_HOLDER;
	}
	
	#if APP_DEBUG   
	printf("\r\nserver IP:");
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
	#endif
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	
  err_code = FLASH_ErasePage( SYSCFG_ADDR & 0xFFFFFFFF);     
	if(err_code != 4)       /*����ʧ��*/
	{
		#if APP_DEBUG
		printf("erase flash error\r\n");
		#endif	
	}
	
	_ulFlashAddr = SYSCFG_ADDR;
	
	for (uint8_t i = 0; i < SYSCFG_DATA_LEN / 2; i++)
	{	
		usTemp = sysCfg.data[2 * i];
		usTemp |= (sysCfg.data[2 * i + 1] << 8);
		err_code = FLASH_ProgramHalfWord(_ulFlashAddr, usTemp);
		if (err_code != FLASH_COMPLETE)
		{
			break;
		}
		
		_ulFlashAddr += 2;
	}
	
	if (err_code == FLASH_COMPLETE)
	{
		mqtt_disconnect();
		vTaskDelay(1000);
		NVIC_SystemReset();
	}
	
	FLASH_Lock();
	
	#if APP_DEBUG
	printf("write flash error\r\n");
	#endif
}

/*
*********************************************************************************************************
 * ��������app_palt_reset
 * ����  ���豸����
 * ����  ��TopicInfo,������Ϣ
 *			 : len�����ⳤ��
 * ����  : ��
*********************************************************************************************************
*/
static void app_palt_reset(uint8_t *TopicInfo , uint16_t len )
{
	uint16_t crcdata;
	uint8_t deal_result;
	uint8_t topicdata[18]={0x0e,0x00,0x4B};

	if(TopicInfo[3] != 0x01 || TopicInfo[2] != 0xB4)
	{
		return;
	}
	
	deal_result = 0x01; 
  memset(&topicdata[3],0,2);                            /*�����豸ID���ֽڲ���*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*����ID*/
	memcpy(&topicdata[11],&deal_result,1);  							/*����ɹ�*/
  memcpy(&topicdata[12],&TopicInfo[len-6],4);           /**/

	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( SOCK_TCPS, RESETACK_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}	
	
	vTaskDelay(1000);
	
	POW_RESET();
}

/*
*********************************************************************************************************
 * ��������app_plat_usMBCRC16
 * ����  ������CRCУ��
 * ����  ��pucFrame,����������
 *			 : usLen�����������ݳ���
 * ����  : CRCУ��ֵ
*********************************************************************************************************
*/
uint16_t app_plat_usMBCRC16( uint8_t * pucFrame, uint16_t usLen )
{
	uint8_t ucCRCHi = 0xFF;
	uint8_t ucCRCLo = 0xFF;
	uint16_t iIndex;
	while( usLen-- )
	{
		iIndex = ucCRCLo ^ *( pucFrame++ );
		ucCRCLo = ( uint8_t )( ucCRCHi ^ aucCRCHi[iIndex] );
		ucCRCHi = aucCRCLo[iIndex];
	}
	return ( uint16_t )( ucCRCHi << 8 | ucCRCLo );
}

/*
*********************************************************************************************************
 * ��������app_mes_fifoinit
 * ����  ��������Ϣ���ݵ�FIFO��ʼ��������дFIFO��ַָ�롢��FIFO��ַָ�롢FIFO��С�Ȳ���
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_palt_fifoinit(void)
{
	mes_fifo.pRxBuf=mes_rxbuf;
	mes_fifo.usRxBufSize=MES_RX_BUF_SIZE;
	mes_fifo.usRxCount=0;
	mes_fifo.usRxRead=0;
	mes_fifo.usRxWrite=0;
}


/*
*********************************************************************************************************
 * ��������app_plat_SendMessage
 * ����  ��������Ϣ���ֻ�
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_SendMessage(void)
{ 
	uint16_t crcdata;
	uint16_t i=0,len=0;
	uint8_t mes[300]={0};
	
	/*�����ݰ�ͷ*/
	while(mes_fifo.usRxCount>0)
	{
		mes[i++]=mes_fifo.pRxBuf[mes_fifo.usRxRead];
		if (++mes_fifo.usRxRead >= mes_fifo.usRxBufSize)
		{
			mes_fifo.usRxRead=0;
		}
		mes_fifo.usRxCount--;
		 
		if(mes[i-1]==0x5a)   /*���ݰ�ͷ*/
		{
			break;  
		}
  }
	
	/*�������*/
	if(mes_fifo.usRxCount==0) 
	{
		return;
	}
	
	memset(mes,0,300);
  for(i=0;i<2;i++)  /*��ȡ����*/
	{
		mes[i]=mes_fifo.pRxBuf[mes_fifo.usRxRead];
		if (++mes_fifo.usRxRead >= mes_fifo.usRxBufSize)
		{
			mes_fifo.usRxRead=0;
		}
		mes_fifo.usRxCount--;
	}
	
	/*���ȼ��*/
	len=mes[0]+mes[1]*256;
	if(len>300)
	{
		#if APP_DEBUG
		printf("send mes len error.\r\n");
	  #endif
		return;
	}
	
	/*��ȡ��Ч����*/
  for(i=0;i<len+2;i++)
	{
		mes[2+i]=mes_fifo.pRxBuf[mes_fifo.usRxRead];
		if (++mes_fifo.usRxRead >= mes_fifo.usRxBufSize)
		{
			mes_fifo.usRxRead=0;
		}
		mes_fifo.usRxCount--;
	}
	
	#if 0
	printf("have parsing data.\r\n");
	for(i=0;i<len+4;i++)
	{
		printf("%2x ",mes[i]);
	}
	printf("\r\n");
	#endif
	
	/*У������*/
	crcdata=app_plat_usMBCRC16(mes,len+2);
	
	/*CRC���*/
	if(((crcdata%256)!=mes[len+2])||((crcdata/256)!=mes[len+3]))
	{
		#if APP_DEBUG
		printf("send mes:CRC error\r\n");
		#endif
	}
	
	app_palt_BandData(mes,len+4);   /*�·���Ϣ*/
}

/*
*********************************************************************************************************
 * ��������app_plat_SportDataPublish
 * ����  ���ϱ��ֻ��˶�����
 * ����  ��bracelet,�ֻ�MAC
 *       : sportdata,�˶����ݣ��Ʋ�����·����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_SportDataPublish(uint8_t *bracelet,uint8_t *sportdata)
{
	uint16_t crcdata;
	static uint32_t SportSerialNumber=0;
	uint8_t topicdata[30]={0x1A,0x00,0x51};
		
	DateTime nowtime;
  get_ntp_time(&nowtime); 
	topicdata[2]=SPORT;               				   /*Msg_id*/
	
	memcpy(&topicdata[3],bracelet,6);			       /*�ֻ�MAC*/
	memset(&topicdata[9],0,2);          				 /*�ֻ�MAC���ֽڲ���*/
	memcpy(&topicdata[11],sportdata,6);			     /*�˶�����*/
	memcpy(&topicdata[17],nowtime.data,7);       /*ʱ���*/
	memcpy(&topicdata[24],(uint8_t *)&SportSerialNumber,4); /*��ˮ��*/
	SportSerialNumber++;
		
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SPORT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
	}
}

/*
*********************************************************************************************************
 * ��������app_plat_HistorySportDataPublish
 * ����  ���ϱ��ֻ���ʷ�˶�����
 * ����  ��bracelet,�ֻ�MAC
 *       : sportdata,�˶����ݣ��Ʋ�����·����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_HistorySportDataPublish(uint8_t *bracelet,uint8_t *sportdata)
{
	uint16_t crcdata;
	DateTime nowtime;
	static uint32_t HistorySportSerialNumber=0;
	uint8_t topicdata[30]={0x1A,0x00,0x51};
		
  get_ntp_time(&nowtime);
	
	topicdata[2]=SPORT;               				   /*Msg_id*/
	
	memcpy(&topicdata[3],bracelet,6);			       /*�ֻ�MAC*/
	memset(&topicdata[9],0,2);          				 /*�ֻ�MAC���ֽڲ���*/
	memcpy(&topicdata[11],sportdata,2);			     /*�˶�����*/
	memcpy(&topicdata[13],&sportdata[4],2);			 /*�˶�����*/
	memcpy(&topicdata[15],&sportdata[2],2);			 /*�˶�����*/
	memcpy(&topicdata[17],nowtime.data,7);       /*ʱ���*/
	memcpy(&topicdata[24],(uint8_t *)&HistorySportSerialNumber,4); /*��ˮ��*/
	HistorySportSerialNumber++;
		
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, HIS_SPORT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * ��������app_plat_HistorySportDataPublish
 * ����  ���ϱ��ֻ���ʷ�˶�����
 * ����  ��bracelet,�ֻ�MAC
 *       : sportdata,�˶����ݣ��Ʋ�����·����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_SleepDataPublish(uint8_t *bracelet,uint8_t *sleepdata)
{
	uint16_t crcdata;
	static uint32_t SleeSerialNumber=0;
	uint8_t topicdata[30]={0x1A,0x00,0x51};

	DateTime nowtime;
  get_ntp_time(&nowtime);
	
	topicdata[2] = SLEEP_DATA;           				 /*Msg_id*/
	
	memcpy(&topicdata[3],bracelet,6);			       /*�ֻ�MAC*/
	memset(&topicdata[9],0,2);          				 /*�ֻ�MAC���ֽڲ���*/
	memcpy(&topicdata[11],sleepdata,6);			     /*˯������*/
	memcpy(&topicdata[17],nowtime.data,7);       /*ʱ���*/
	memcpy(&topicdata[24],(uint8_t *)&SleeSerialNumber,4); /*��ˮ��*/
	SleeSerialNumber++;
		
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SLEEP_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * ��������app_plat_SHTDataPublish
 * ����  ����ʪ�������ϴ�
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_SHTDataPublish(void)
{
	uint16_t crcdata;
	static uint32_t SHTDataSerialNumber = 0;
	
	uint8_t Temp = 0;
  uint8_t Hum = 0;	
	uint8_t topicdata[19]={0x0f,0x00,0x55};
	
	SHT_GetValue(&Temp, &Hum);
	
	topicdata[2] = SHT_DATA;                              /*Msg_id*/
  memset(&topicdata[3],0,2);                            /*�����豸ID���ֽڲ���*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*����ID*/
	memcpy(&topicdata[11],&Temp,1);						        		/*�¶�*/
	memcpy(&topicdata[12],&Hum,1);						   	        /*ʪ��*/
	
	memcpy(&topicdata[13],(uint8_t *)&SHTDataSerialNumber,4); /*��ˮ��*/
	SHTDataSerialNumber++;
	
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[17],(uint8_t *)&crcdata,2);   
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SHT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}
}

/*
*********************************************************************************************************
 * ��������app_plat_HeartratePublish
 * ����  ���ϱ��ֻ���������
 * ����  ��bracelet,�ֻ�MAC
 *       : heartrate,��������
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_HeartratePublish(uint8_t *bracelet,uint8_t heartrate)
{
	uint16_t crcdata;
	static uint32_t HealthSerialNumber=0;
	uint8_t topicdata[25]={0x15,0x00,0x52};
	
	DateTime nowtime;
  get_ntp_time(&nowtime);
	
	topicdata[2]=HEALTH;                          /*Msg_id*/
	
	memcpy(&topicdata[3],bracelet,6);			        /*�ֻ�MAC*/
	memset(&topicdata[9],0,2);          				  /*�ֻ�MAC���ֽڲ���*/
	memcpy(&topicdata[11],&heartrate,1);          /*��������*/
	memcpy(&topicdata[12],nowtime.data,7);        /*ʱ���*/
	memcpy(&topicdata[19],(uint8_t *)&HealthSerialNumber,4); /*��ˮ��*/
	HealthSerialNumber++;
	
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[23],(uint8_t *)&crcdata,2);  
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, HEALTH_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * ��������app_plat_LocationPublish
 * ����  ���ϱ��ֻ�λ������
 * ����  ��bracelet,�ֻ�MAC
 *       : location,λ������
 *       : beaconelectricity���ű����
 *       : beastation��������վ�豸MAC
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_LocationPublish(uint8_t *bracelet,uint8_t *location,uint8_t beaconelectricity,uint8_t *beastation)
{
	uint16_t crcdata;
	static uint32_t LocationSerialNumber=0;
	
	uint8_t beacon_warn_mac[8] = {0x00};
	uint8_t beacon_warn_para[2] = {0x01};
	uint8_t topicdata[46]={0x2a,0x00,0x53};

	DateTime nowtime;
  get_ntp_time(&nowtime);
		
	topicdata[2]=LOCATION;                                /*Msg_id*/
	
	memcpy(&topicdata[3],beastation,8);                   /*������վ�豸MAC*/
  memset(&topicdata[11],0,2);                           /*�����豸ID���ֽڲ���*/
	memcpy(&topicdata[13],sysCfg.parameter.client_mac,6); /*����ID*/
	memcpy(&topicdata[19],bracelet,6);						        /*�ֻ�MAC*/
	memset(&topicdata[25],0,2);          								  /*�ֻ�MAC���ֽڲ���*/
	
	memcpy(&topicdata[27],location,1);                    /*�ֻ�RSSI*/
	memcpy(&topicdata[28],location+7,4);                  /*λ������*/
	memcpy(&topicdata[32],&beaconelectricity,1);          /*�ű����*/
	memcpy(&topicdata[33],nowtime.data,7);                /*ʱ���*/
	
	memcpy(&topicdata[40],(uint8_t *)&LocationSerialNumber,4); /*��ˮ��*/
	LocationSerialNumber++;
	
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[44],(uint8_t *)&crcdata,2);   
	
	/*�ϱ�����*/
  if(mqtt_publish( sysCfg.parameter.data_socket, LOCATION_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}
	
	/*�ű��ѹ�澯*/
	if((((float)beaconelectricity)/256.0*3.6) < BEACON_WARN_VOLTAGE)
	{
		beacon_warn_mac[0] = location[7];
		beacon_warn_para[1] = beaconelectricity;
		ErrorLog(BEACON,beacon_warn_mac,WARN,ELECT_WARN,1,beacon_warn_para); /*�豸�澯*/
	}
}

/*
*********************************************************************************************************
 * ��������app_plat_AttendancePublish
 * ����  ���ϱ��ֻ���������
 * ����  ��bracelet,�ֻ�MAC
 *       : location,λ������
 *       : beastation��������վ�豸MAC
 *       : inoutflag,������־,1����2��
 * ����  : ��
*********************************************************************************************************
*/
//void app_plat_AttendancePublish(uint8_t *bracelet,uint8_t *location,uint8_t *beastation,uint8_t inoutflag)
//{
//	uint16_t crcdata;
//	static uint32_t AttendancePSerialNumber=0;
//	uint8_t topicdata[46]={0x2a,0x00,0x53};
//	
//	topicdata[2] = ATTEN;                                 /*Msg_id*/
//	
//	memcpy(&topicdata[3],beastation,8);                   /*������վ�豸MAC*/
//  memset(&topicdata[11],0,2);                           /*�����豸ID���ֽڲ���*/
//	memcpy(&topicdata[13],sysCfg.parameter.client_mac,6); /*����ID*/
//	memcpy(&topicdata[19],bracelet,6);						        /*�ֻ�MAC*/
//	memset(&topicdata[25],0,2);          								  /*�ֻ�MAC���ֽڲ���*/
//	
//	memcpy(&topicdata[27],location,1);                    /*λ������*/
//	memcpy(&topicdata[28],location+7,4);                  /*λ������*/
//	
//	memcpy(&topicdata[32],&inoutflag,1); 
//	memcpy(&topicdata[33],nowdate.data,7);                /*ʱ���*/
//	memcpy(&topicdata[40],(uint8_t *)&AttendancePSerialNumber,4); 		/*��ˮ��*/
//	AttendancePSerialNumber++;
//	
//	/*CRCУ��*/
//	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
//	memcpy(&topicdata[44],(uint8_t *)&crcdata,2);   
//	
//	/*�ϱ�����*/
//  if(mqtt_publish( sysCfg.parameter.data_socket, ATTENDANCE_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
//	{
//		
//	}
//}

/*
*********************************************************************************************************
 * ��������app_plat_BraceletInfoPublish
 * ����  ���ϱ��ֻ���Ϣ
 * ����  ��bracelet,�ֻ�MAC
 *       : braceletelectricity���ֻ�����
 * ����  : ��
*********************************************************************************************************
*/
void app_plat_BraceletInfoPublish(uint8_t *bracelet,uint8_t braceletelectricity)
{
	uint16_t crcdata;
	static uint32_t BraceletInfoSerialNumber=0;
	uint8_t beacon_warn_para[2] = {0x01};
	uint8_t topicdata[18]={0x0e,0x00,0x54};

	topicdata[2]=BASEINFOR;                              /*Msg_id*/	
	
	memcpy(&topicdata[3],bracelet,6);						         /*�ֻ�MAC*/
	memset(&topicdata[9],0,2);          								 /*�ֻ�MAC���ֽڲ���*/
	memcpy(&topicdata[11],&braceletelectricity,1);       /*�ֻ�����*/
	memcpy(&topicdata[12],(uint8_t *)&BraceletInfoSerialNumber,4);  /*��ˮ��*/
	BraceletInfoSerialNumber++;
	
	/*CRCУ��*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2); 
	
	/*�ϱ�����*/	
  if(mqtt_publish( sysCfg.parameter.data_socket, BASEINFOR_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
	
	}
	
	if(braceletelectricity < BRACELET_WARN_ELECT)
	{
		beacon_warn_para[1] = braceletelectricity;
		ErrorLog(BRACELET,&topicdata[3],WARN,ELECT_WARN,1,beacon_warn_para); /*�豸�澯*/
	}
}


/*
*********************************************************************************************************
 * ��������app_palt_DealPlatinfo
 * ����  ������ƽ̨��Ϣ,�����Զ��ֻ��������ԡ�������Ϣ��ʱ��
 * ����  ��TopicInfo,ƽ̨��Ϣ
 *			 : Len, ��Ϣ����
 *			 : topic, ���յ���Ϣ������
 * ����  : ��
*********************************************************************************************************
*/

static void app_palt_Datadeal(uint8_t *TopicInfo,uint16_t Len,char * topic)
{
	uint16_t crcdata; 
	app_plat_topic nState;
  							
  /*���ݳ��ȼ��*/	
	if((TopicInfo[0]+TopicInfo[1]*256)!=Len-4)
	{
		#if APP_DEBUG
		printf("Topic len error");
		printf("TopicInfo[0]+TopicInfo[1]*256=%d,len=%d\r\n",TopicInfo[0]+TopicInfo[1]*256,Len);
		printf("data:");
		for(uint8_t i = 0; i < Len; i++)
		{
			printf("%02x ",TopicInfo[i]);
		}
		printf("\r\n");
	  #endif
		return;
	}
	
	/*CRCУ����*/
	crcdata=app_plat_usMBCRC16(TopicInfo,Len-2);
	if((TopicInfo[Len-1]!=crcdata/256)||(TopicInfo[Len-2]!=crcdata%256))
	{
		#if APP_DEBUG
		printf("CRC error\r\n");
		#endif
		return;
	}
	
	if(app_plat_GetRespone(TopicInfo , topic , &nState ))
	{
		switch(nState)
		{
			/*���Ͷ���Ϣ*/
			case PLAT_RESPONSE_SEND_MESSAGE:
				if((TopicInfo[0]+TopicInfo[1]*256-29)>230)
				{		
					return;
				}
				#if APP_DEBUG
				printf("receive meseage\r\n");
				#endif
				
				/*������Ϣ���ݵ�������*/
				app_palt_WriteFifoData(TopicInfo,Len);
				break;
			
			/*�����豸����*/
			case PLAT_RESPONSE_DEVUPDATE:
				IWDG_Feed();
			  bsp_ota_UpdataGateway(Len,TopicInfo);  
				break;
			
			 /*�����ֻ�����*/
			case PLAT_RESPONSE_BRAUPDATE:
				IWDG_Feed();
			  bsp_ota_UpdataBracelet(Len,TopicInfo);  
				break;
			
			/*�����豸������ʼָ��*/
			case PLAT_RESPONSE_BLEUPDATE:
			  app_palt_UpdataBle(TopicInfo);  
				break;
			
			/*�����ű����*/
//	    case PLAT_RESPONSE_BEAMANAGEMENT:
//				app_palt_BeaconManagement(TopicInfo);
//				break;
			
			/*�����û���Ϣ*/
			case PLAT_RESPONSE_USERINFOSET:
				app_palt_userinfoset( TopicInfo , Len );
				break;
			
			/*�����豸������Ϣ*/
			case PLAT_RESPONSE_DEVCFGSET:
				#if APP_DEBUG
				printf("receive user info.");
			  #endif
				app_palt_devcfgset( TopicInfo , Len );
				break;
			
			/*�豸Ӳ������*/
			case PLAT_RESPONSE_RESET:
				printf("PLAT_RESPONSE_RESET");
				app_palt_reset( TopicInfo , Len );
				break;
			
			default:
				break;
		}
	}
}

/*
*********************************************************************************************************
 * ��������app_palt_Receicedata
 * ����  ������ƽ̨����
 * ����  ��plat_report_t,ƽ̨��Ϣ�ṹ��
 * ����  : ��
*********************************************************************************************************
*/
void app_palt_Receicedata(plat_report_t * report_t)
{
	uint8_t  len;
	uint8_t  dup;					//�ظ���־
	uint8_t  ack[30];			//��ִ
  uint8_t  retained;		//������־
  uint8_t  *payload_in; //��������
  
	uint16_t msgid;				//��ϢID
	
	int qos;							//��Ϣ����
	int payloadlen_in;		//���س���
	MQTTString receivedTopic;
	
	/*������Ϣ����*/
	MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
	&payload_in, &payloadlen_in, report_t->payload_data , PLAT_MAX_DATA_LEN );				

	/*��Ϣ��ִ ���ĵ�QOS=1��Ҫ��ִ*/
	len = MQTTSerialize_puback(ack, sizeof(ack),msgid);
	transport_sendPacketBuffer(report_t->socket,ack,len);

	if(0 == strncmp(receivedTopic.lenstring.data,gateupdata_topic,strlen(gateupdata_topic))
	&&(0x01 == payload_in[3]+payload_in[4]*256))
	{
		report_t->gateway_updata_flag = GAT_UPDATA_START;
	}

	if(0 == strncmp(receivedTopic.lenstring.data,bleupdata_topic,strlen(bleupdata_topic)))
	{
		report_t->ble_updata_flag = BLE_UPDATA_START;
	}
	
	if(0 == strncmp(receivedTopic.lenstring.data,bracelet_topic,strlen(bracelet_topic))&&(0x01 == payload_in[3]+payload_in[4]*256))
	{
		report_t->bracelet_updata_flag = BRA_UPDATA_START;
	}
		
	app_palt_Datadeal(payload_in,payloadlen_in,receivedTopic.lenstring.data);	
}

void app_palt_Connectack(plat_report_t * report_t)
{
	uint8_t sessionPresent, connack_rc;
	
	if (MQTTDeserialize_connack(&sessionPresent, &connack_rc,  report_t->payload_data , PLAT_MAX_DATA_LEN) != 1 || (connack_rc != 0) )
	{		
		#if APP_DEBUG
		printf("Unable to connect, return code %d\n\r", connack_rc); 
		#endif				
    bsp_LedOff(2);	
		sysCfg.parameter.connect_state = DISCONNECT;
    if(report_t->socket == SOCK_TCPS)		
		{
			while(1);	
    }			
		else  /*������socket�����쳣ָʾ*/
		{
			bsp_LedOn(4);
			vTaskDelay(1000);
			bsp_LedOff(4);
		}
	}
	else
	{
		#if APP_DEBUG
		printf("%s:MQTT connect OK\r\n",(report_t->socket?"TCP":"TCPS"));	
		#endif
		sysCfg.parameter.connect_state = CONNECT;
	}
}

/*
*********************************************************************************************************
 * ��������app_palt_Reportparse
 * ����  ������ƽ̨���ݰ�
 * ����  ��plat_report_t,ƽ̨��Ϣ�ṹ��
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
*********************************************************************************************************
*/
ErrorStatus app_palt_Reportparse(plat_report_t * report_t)
{
	uint16_t len = 0;
	uint8_t reg_status = 0;
	
	reg_status = getIR();               	/*��ȡ�жϱ�־�Ĵ���*/
	setIR(reg_status);                  	/*��д����жϱ�־*/
	reg_status = getSIR();              	/*��ȡ�˿��жϱ�־�Ĵ���*/	
	setSIR(reg_status);										/*��д����˿��жϱ�־�Ĵ���*/
	
	//printf("reg_status��%02x\r\n",reg_status);
	
	/*���������˿��¼�*/
	if((reg_status & (1 << SOCK_TCPS)) == (1 << SOCK_TCPS))     
	{
		report_t->socket = SOCK_TCPS;
		reg_status = getSn_IR(SOCK_TCPS); 	/*��ȡSocket�жϱ�־�Ĵ���*/
		setSn_IR(SOCK_TCPS,reg_status);   	/*��д����ж�*/
		if(reg_status & Sn_IR_RECV)         /*Socket���յ�����,��������S_rx_process()����*/
		{	
			len = getSn_RX_RSR(SOCK_TCPS);		/*��ȡW5500���ջ��������ݴ�С*/
			if(0 == len) 
			{
				return ERROR;
			}
			/*��ȡW5500���ջ���������*/
			report_t->evt_id = MQTTPacket_read(report_t->payload_data, PLAT_MAX_DATA_LEN , transport_getdata0);
			return SUCCESS;
		}
	}

	/*�������������˿��¼�*/
	if((reg_status & (1 << SOCK_TCP)) == (1 << SOCK_TCP))     
	{
		report_t->socket = SOCK_TCP;
		reg_status = getSn_IR(SOCK_TCP);  	/*��ȡSocket�жϱ�־�Ĵ���*/
		setSn_IR(SOCK_TCP,reg_status);    	/*��д����ж�*/
		if(reg_status & Sn_IR_RECV)         /*Socket���յ�����,��������S_rx_process()����*/
		{	
			len = getSn_RX_RSR(SOCK_TCP);			/*��ȡW5500���ջ��������ݴ�С*/
			if(0 == len) 
			{
				return ERROR;
			}
			/*��ȡW5500���ջ���������*/
			report_t->evt_id = MQTTPacket_read(report_t->payload_data, PLAT_MAX_DATA_LEN , transport_getdata1);
			return SUCCESS;
		}
	}
	
	/*BLE�����˿��¼�*/
	else if((reg_status & (1 << SOCK_BLE)) == (1 << SOCK_BLE))
	{
		report_t->socket = SOCK_BLE;
		reg_status = getSn_IR(SOCK_BLE);
		setSn_IR(SOCK_BLE,reg_status);  
		if(reg_status & Sn_IR_RECV)  
		{
			len = getSn_RX_RSR(SOCK_BLE);
			if(0 == len) 
			{
				return ERROR;
			}
		}
		return SUCCESS;
	}
	
	return ERROR;
}

