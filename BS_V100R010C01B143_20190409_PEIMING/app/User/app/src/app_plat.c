#include "includes.h"

static app_plat_fifo mes_fifo;
static uint8_t mes_rxbuf[MES_RX_BUF_SIZE];

/****************************订阅主题******************************/
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
 * 函数名：app_palt_WriteFifoData
 * 描述  ：消息数据存入缓存中
 * 输入  ：p，待存储的数据
 *       : len，待存储数据长度
 * 返回  : 无
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
		/*如果待读取的数据总数小于FIFO的大小,则写入*/
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
 * 函数名：app_palt_UpdataBle
 * 描述  ：开始升级蓝牙
 * 输入  ：TopicInfo,主题信息
 * 返回  : 无
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
	
	memcpy((uint8_t *)&port,&TopicInfo[15],2);  /*获取端口号*/
	
	app_nrf_UpdataBle(&TopicInfo[3]);           /*发送开始升级主题*/
			 
	topicdata[2] = BLEUPDATACK;      
  memset(&topicdata[3],0x00,2);	
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);
	memcpy(&topicdata[12],(uint8_t *)&UpdataBleSerialNumber,4); /*流水号*/
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
	
	/*新建一个Socket并绑定本地端口5000*/
	ret = socket(SOCK_BLE,Sn_MR_TCP,5000,0x00);
	if(ret != SOCK_BLE)
	{
		topicdata[11] = 0x00;
		#if APP_DEBUG
		printf("%d:Socket Error\r\n",SOCK_BLE);
		#endif
	}

	/*连接TCP服务器*/
	ret = connect(SOCK_BLE,ble_ip,port); 
	if(ret != SOCK_OK)
	{
		topicdata[11] = 0x00;
		#if APP_DEBUG
		printf("%d:Socket Connect Error\r\n",SOCK_BLE);
		#endif
	}
		
	USART_Cmd(USART3, DISABLE);		
	
	USART_InitStructure.USART_BaudRate = 9600;	/* 波特率 */
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	USART_Cmd(USART3, ENABLE);		/* 使能串口 */

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
 * 函数名：app_palt_BandData
 * 描述  ：手环通信，下发SMS消息
 * 输入  ：TopicInfo,主题信息
 * 	     ：Len，主题信息总长度
 * 返回  : 无
*********************************************************************************************************
 */
static void app_palt_BandData(uint8_t *TopicInfo,uint16_t  Len)
{
  uint16_t crcdata;
	uint8_t backdata[26]={0x16,0x00,0x1A};
	
	backdata[2] = BANDDATABACK;
	memcpy(&backdata[3],&TopicInfo[11],8);         /*手环MAC（小端）*/
	memcpy(&backdata[11],&TopicInfo[19],8);        /*消息发送时间戳*/
	memcpy(&backdata[20],&TopicInfo[Len-6],4);     /*流水号*/
		
  /*发起连接 传入参数包含读头的ID*/
	if(SUCCESS != app_nrf_ConnectBle(&TopicInfo[3],&TopicInfo[11]))
	{
		#if APP_DEBUG
		printf("app_palt_BandData:Link bracelet fail.\r\n");
		#endif
		backdata[19]=0x01;    
		crcdata=app_plat_usMBCRC16(backdata,24);       /*CRC校验*/
		memcpy(&backdata[24],(uint8_t *)&crcdata,2);		
    mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);

		if(ERROR == app_nrf_DisconnectBle(&TopicInfo[3]))
		{
			#if APP_DEBUG
			printf("app_palt_BandData:disconnet error.\r\n");
			#endif			
		}
    	
    ErrorLog(BRACELET,&TopicInfo[11],WARN,MESEG_WARN,0,NULL); /*设备告警*/
		return;    	        /*连接失败*/	
	} 
	
	vTaskDelay(1000);	
	if(SUCCESS == app_nrf_LeaveMessage(&TopicInfo[3],&TopicInfo[27],Len-33)) 
	{
		#if APP_DEBUG
		printf("Leave Message OK.\r\n");
		#endif
		backdata[19]=0x03;  /*手环收到*/
		crcdata=app_plat_usMBCRC16(backdata,24);       /*CRC校验*/
		memcpy(&backdata[24],(uint8_t *)&crcdata,2);	
		mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);

		vTaskDelay(1000);
		return;
	}  
	
	backdata[19]=0x02; 												  /*手环未收到消息*/
	crcdata=app_plat_usMBCRC16(backdata,24);    /*CRC校验*/
  memcpy(&backdata[24],(uint8_t *)&crcdata,2);	
	mqtt_publish( sysCfg.parameter.data_socket , SMSBCD_TOPIC , (char *)backdata , 26);
  
	#if APP_DEBUG
	printf("app_palt_BandData:Leave a message fail.\r\n");
	#endif
	if(ERROR == app_nrf_DisconnectBle(&TopicInfo[3]))
	{
	}	
	
	ErrorLog(BRACELET,&TopicInfo[11],WARN,MESEG_WARN,0,NULL);  /*设备告警*/
	vTaskDelay(1000);
}

/*
*********************************************************************************************************
 * 函数名：app_plat_GetRespone
 * 描述  ：获取消息类型
 * 输入  ：topic,消息主题
 *			 : pRespone，存放消息类型
 * 返回  : 
 *       ：SUCCESS,是待处理的消息类型；
 *       ：ERROR，不是待处理的消息类型
*********************************************************************************************************
*/
static ErrorStatus app_plat_GetRespone(uint8_t *TopicInfo,char *topic,app_plat_topic* pRespone)
{ 
	#if 0
	printf("Receive topic:%s\r\n",topic);
	#endif
	
   /*下发消息主题*/
	if( NULL!=strstr( topic, "/notice/sms"))
	{			
    *pRespone = PLAT_RESPONSE_SEND_MESSAGE;
		return SUCCESS;
	}

	/*考勤信标下发*/
	else if( strstr( topic, "/management/beacon"))
	{
		*pRespone = PLAT_RESPONSE_BEAMANAGEMENT;
		return SUCCESS;
	}
	
	/*蓝牙设备升级*/
	else if( strstr( topic, "/update/ble"))
	{
		*pRespone = PLAT_RESPONSE_BLEUPDATE;
		return SUCCESS;
	}

	/*网关设备升级*/
	else if( strstr( topic, "/update/gateway"))
	{
		*pRespone = PLAT_RESPONSE_DEVUPDATE;
		return SUCCESS;
	}
		/*蓝牙手环升级*/
	else if( strstr( topic, "/update/bracelet"))
	{
		*pRespone = PLAT_RESPONSE_BRAUPDATE;
		return SUCCESS;
	}
	
	/*用户信息*/
	else if( strstr( topic, "/user_info/set"))
	{
		*pRespone = PLAT_RESPONSE_USERINFOSET;
		return SUCCESS;
	}

	/*配置设备联网参数*/
	else if( strstr( topic, "/devcfgpara/set"))
	{
		*pRespone = PLAT_RESPONSE_DEVCFGSET;
		return SUCCESS;
	}

	/*蓝牙网关硬件复位*/
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
 * 函数名：app_palt_BeaconManagement
 * 描述  ：考勤信标管理
 * 输入  ：
 *       : TopicInfo,主题信息
 * 返回  : 无
*********************************************************************************************************
*/
//static void app_palt_BeaconManagement(uint8_t *TopicInfo)
//{
//	/*第一个信标在门内侧，第二个信标在门外侧*/
//	switch(TopicInfo[3])
//	{
//		/*获取信标列表*/
//		case 0x00:
//			app_flash_GetBeaconList(&TopicInfo[12]);
//			break;
//		
//		/*插入一组信标*/
//		case 0x01:
//			app_flash_AddBeacon(&TopicInfo[4],&TopicInfo[8]);
//			break;
//		
//		/*删除一组信标*/
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
 * 函数名：app_palt_userinfoset
 * 描述  ：用户信息设置
 * 输入  ：
 *       : TopicInfo,主题信息
 *       : len,主题信息
 * 返回  : 无
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
	
	userlen=len-33+1;                        									/*用户信息长度*/
	memcpy( userinfo , TopicInfo , 27 );
	memcpy( &userinfo[27] , (uint8_t *)&userlen , 2 );
	userinfo[29]=1;                        						  			/*标志位*/
	memcpy( &userinfo[30] , &TopicInfo[27] , len-33 );
	crcdata=app_plat_usMBCRC16( &userinfo[27] , len-33+3 );
	memcpy( &userinfo[30+len-33], (uint8_t *)&crcdata , 2 );  
	memcpy( &userinfo[32+userlen], &TopicInfo[len-6] , 4 );   /*原信息中的流水号*/
  userlen=TopicInfo[0]+TopicInfo[1]*256+5;
	memcpy( userinfo , (uint8_t *)&userlen , 2 );
	
	/*CRC校验*/
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
	
	memcpy(&ask[3],&TopicInfo[11],8);               /*手环MAC*/
	memcpy(&ask[11],&TopicInfo[19],8);
	ask[19]=0x03;        														
	/*手环收到*/
	memcpy(&ask[20],&TopicInfo[len-6],4);
		
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(ask,ask[1]*256+ask[0]+2);
	memcpy(&ask[24],(uint8_t *)&crcdata,2);  
	
	/*返回数据*/
  mqtt_publish( sysCfg.parameter.data_socket , USERINFO_TOPIC , (char *)ask , ask[1]*256+ask[0]+4);
	
	app_palt_WriteFifoData(userinfo,userinfo[1]*256+userinfo[0]+4);
}

/*
*********************************************************************************************************
 * 函数名：app_palt_devcfgset
 * 描述  ：连接第第三方服务器信息设置
 * 输入  ：
 *       : TopicInfo,主题信息
 *       : len,主题信息
 * 返回  : 无
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
	if(err_code != 4)       /*擦除失败*/
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
 * 函数名：app_palt_reset
 * 描述  ：设备重启
 * 输入  ：TopicInfo,主题信息
 *			 : len，主题长度
 * 返回  : 无
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
  memset(&topicdata[3],0,2);                            /*网关设备ID低字节补齐*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*网关ID*/
	memcpy(&topicdata[11],&deal_result,1);  							/*处理成功*/
  memcpy(&topicdata[12],&TopicInfo[len-6],4);           /**/

	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( SOCK_TCPS, RESETACK_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}	
	
	vTaskDelay(1000);
	
	POW_RESET();
}

/*
*********************************************************************************************************
 * 函数名：app_plat_usMBCRC16
 * 描述  ：数据CRC校验
 * 输入  ：pucFrame,待处理数据
 *			 : usLen，待处理数据长度
 * 返回  : CRC校验值
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
 * 函数名：app_mes_fifoinit
 * 描述  ：保存消息数据的FIFO初始化：设置写FIFO地址指针、读FIFO地址指针、FIFO大小等参数
 * 输入  ：无
 * 返回  : 无
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
 * 函数名：app_plat_SendMessage
 * 描述  ：发送消息给手环
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_plat_SendMessage(void)
{ 
	uint16_t crcdata;
	uint16_t i=0,len=0;
	uint8_t mes[300]={0};
	
	/*读数据包头*/
	while(mes_fifo.usRxCount>0)
	{
		mes[i++]=mes_fifo.pRxBuf[mes_fifo.usRxRead];
		if (++mes_fifo.usRxRead >= mes_fifo.usRxBufSize)
		{
			mes_fifo.usRxRead=0;
		}
		mes_fifo.usRxCount--;
		 
		if(mes[i-1]==0x5a)   /*数据包头*/
		{
			break;  
		}
  }
	
	/*缓存读空*/
	if(mes_fifo.usRxCount==0) 
	{
		return;
	}
	
	memset(mes,0,300);
  for(i=0;i<2;i++)  /*读取长度*/
	{
		mes[i]=mes_fifo.pRxBuf[mes_fifo.usRxRead];
		if (++mes_fifo.usRxRead >= mes_fifo.usRxBufSize)
		{
			mes_fifo.usRxRead=0;
		}
		mes_fifo.usRxCount--;
	}
	
	/*长度检查*/
	len=mes[0]+mes[1]*256;
	if(len>300)
	{
		#if APP_DEBUG
		printf("send mes len error.\r\n");
	  #endif
		return;
	}
	
	/*读取有效数据*/
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
	
	/*校验数据*/
	crcdata=app_plat_usMBCRC16(mes,len+2);
	
	/*CRC检测*/
	if(((crcdata%256)!=mes[len+2])||((crcdata/256)!=mes[len+3]))
	{
		#if APP_DEBUG
		printf("send mes:CRC error\r\n");
		#endif
	}
	
	app_palt_BandData(mes,len+4);   /*下发消息*/
}

/*
*********************************************************************************************************
 * 函数名：app_plat_SportDataPublish
 * 描述  ：上报手环运动数据
 * 输入  ：bracelet,手环MAC
 *       : sportdata,运动数据，计步、卡路里、里程
 * 返回  : 无
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
	
	memcpy(&topicdata[3],bracelet,6);			       /*手环MAC*/
	memset(&topicdata[9],0,2);          				 /*手环MAC低字节补齐*/
	memcpy(&topicdata[11],sportdata,6);			     /*运动数据*/
	memcpy(&topicdata[17],nowtime.data,7);       /*时间戳*/
	memcpy(&topicdata[24],(uint8_t *)&SportSerialNumber,4); /*流水号*/
	SportSerialNumber++;
		
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SPORT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_HistorySportDataPublish
 * 描述  ：上报手环历史运动数据
 * 输入  ：bracelet,手环MAC
 *       : sportdata,运动数据，计步、卡路里、里程
 * 返回  : 无
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
	
	memcpy(&topicdata[3],bracelet,6);			       /*手环MAC*/
	memset(&topicdata[9],0,2);          				 /*手环MAC低字节补齐*/
	memcpy(&topicdata[11],sportdata,2);			     /*运动数据*/
	memcpy(&topicdata[13],&sportdata[4],2);			 /*运动数据*/
	memcpy(&topicdata[15],&sportdata[2],2);			 /*运动数据*/
	memcpy(&topicdata[17],nowtime.data,7);       /*时间戳*/
	memcpy(&topicdata[24],(uint8_t *)&HistorySportSerialNumber,4); /*流水号*/
	HistorySportSerialNumber++;
		
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, HIS_SPORT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_HistorySportDataPublish
 * 描述  ：上报手环历史运动数据
 * 输入  ：bracelet,手环MAC
 *       : sportdata,运动数据，计步、卡路里、里程
 * 返回  : 无
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
	
	memcpy(&topicdata[3],bracelet,6);			       /*手环MAC*/
	memset(&topicdata[9],0,2);          				 /*手环MAC低字节补齐*/
	memcpy(&topicdata[11],sleepdata,6);			     /*睡眠数据*/
	memcpy(&topicdata[17],nowtime.data,7);       /*时间戳*/
	memcpy(&topicdata[24],(uint8_t *)&SleeSerialNumber,4); /*流水号*/
	SleeSerialNumber++;
		
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[28],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SLEEP_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_SHTDataPublish
 * 描述  ：温湿度数据上传
 * 输入  ：无
 * 返回  : 无
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
  memset(&topicdata[3],0,2);                            /*网关设备ID低字节补齐*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);  /*网关ID*/
	memcpy(&topicdata[11],&Temp,1);						        		/*温度*/
	memcpy(&topicdata[12],&Hum,1);						   	        /*湿度*/
	
	memcpy(&topicdata[13],(uint8_t *)&SHTDataSerialNumber,4); /*流水号*/
	SHTDataSerialNumber++;
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[17],(uint8_t *)&crcdata,2);   
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, SHT_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_HeartratePublish
 * 描述  ：上报手环心率数据
 * 输入  ：bracelet,手环MAC
 *       : heartrate,心率数据
 * 返回  : 无
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
	
	memcpy(&topicdata[3],bracelet,6);			        /*手环MAC*/
	memset(&topicdata[9],0,2);          				  /*手环MAC低字节补齐*/
	memcpy(&topicdata[11],&heartrate,1);          /*心率数据*/
	memcpy(&topicdata[12],nowtime.data,7);        /*时间戳*/
	memcpy(&topicdata[19],(uint8_t *)&HealthSerialNumber,4); /*流水号*/
	HealthSerialNumber++;
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[23],(uint8_t *)&crcdata,2);  
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, HEALTH_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{

	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_LocationPublish
 * 描述  ：上报手环位置数据
 * 输入  ：bracelet,手环MAC
 *       : location,位置数据
 *       : beaconelectricity，信标电量
 *       : beastation，蓝牙基站设备MAC
 * 返回  : 无
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
	
	memcpy(&topicdata[3],beastation,8);                   /*蓝牙基站设备MAC*/
  memset(&topicdata[11],0,2);                           /*网关设备ID低字节补齐*/
	memcpy(&topicdata[13],sysCfg.parameter.client_mac,6); /*网关ID*/
	memcpy(&topicdata[19],bracelet,6);						        /*手环MAC*/
	memset(&topicdata[25],0,2);          								  /*手环MAC低字节补齐*/
	
	memcpy(&topicdata[27],location,1);                    /*手环RSSI*/
	memcpy(&topicdata[28],location+7,4);                  /*位置数据*/
	memcpy(&topicdata[32],&beaconelectricity,1);          /*信标电量*/
	memcpy(&topicdata[33],nowtime.data,7);                /*时间戳*/
	
	memcpy(&topicdata[40],(uint8_t *)&LocationSerialNumber,4); /*流水号*/
	LocationSerialNumber++;
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[44],(uint8_t *)&crcdata,2);   
	
	/*上报数据*/
  if(mqtt_publish( sysCfg.parameter.data_socket, LOCATION_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
		
	}
	
	/*信标低压告警*/
	if((((float)beaconelectricity)/256.0*3.6) < BEACON_WARN_VOLTAGE)
	{
		beacon_warn_mac[0] = location[7];
		beacon_warn_para[1] = beaconelectricity;
		ErrorLog(BEACON,beacon_warn_mac,WARN,ELECT_WARN,1,beacon_warn_para); /*设备告警*/
	}
}

/*
*********************************************************************************************************
 * 函数名：app_plat_AttendancePublish
 * 描述  ：上报手环考勤数据
 * 输入  ：bracelet,手环MAC
 *       : location,位置数据
 *       : beastation，蓝牙基站设备MAC
 *       : inoutflag,进出标志,1进，2出
 * 返回  : 无
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
//	memcpy(&topicdata[3],beastation,8);                   /*蓝牙基站设备MAC*/
//  memset(&topicdata[11],0,2);                           /*网关设备ID低字节补齐*/
//	memcpy(&topicdata[13],sysCfg.parameter.client_mac,6); /*网关ID*/
//	memcpy(&topicdata[19],bracelet,6);						        /*手环MAC*/
//	memset(&topicdata[25],0,2);          								  /*手环MAC低字节补齐*/
//	
//	memcpy(&topicdata[27],location,1);                    /*位置数据*/
//	memcpy(&topicdata[28],location+7,4);                  /*位置数据*/
//	
//	memcpy(&topicdata[32],&inoutflag,1); 
//	memcpy(&topicdata[33],nowdate.data,7);                /*时间戳*/
//	memcpy(&topicdata[40],(uint8_t *)&AttendancePSerialNumber,4); 		/*流水号*/
//	AttendancePSerialNumber++;
//	
//	/*CRC校验*/
//	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
//	memcpy(&topicdata[44],(uint8_t *)&crcdata,2);   
//	
//	/*上报数据*/
//  if(mqtt_publish( sysCfg.parameter.data_socket, ATTENDANCE_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
//	{
//		
//	}
//}

/*
*********************************************************************************************************
 * 函数名：app_plat_BraceletInfoPublish
 * 描述  ：上报手环信息
 * 输入  ：bracelet,手环MAC
 *       : braceletelectricity，手环电量
 * 返回  : 无
*********************************************************************************************************
*/
void app_plat_BraceletInfoPublish(uint8_t *bracelet,uint8_t braceletelectricity)
{
	uint16_t crcdata;
	static uint32_t BraceletInfoSerialNumber=0;
	uint8_t beacon_warn_para[2] = {0x01};
	uint8_t topicdata[18]={0x0e,0x00,0x54};

	topicdata[2]=BASEINFOR;                              /*Msg_id*/	
	
	memcpy(&topicdata[3],bracelet,6);						         /*手环MAC*/
	memset(&topicdata[9],0,2);          								 /*手环MAC低字节补齐*/
	memcpy(&topicdata[11],&braceletelectricity,1);       /*手环电量*/
	memcpy(&topicdata[12],(uint8_t *)&BraceletInfoSerialNumber,4);  /*流水号*/
	BraceletInfoSerialNumber++;
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(topicdata,topicdata[1]*256+topicdata[0]+2);
	memcpy(&topicdata[16],(uint8_t *)&crcdata,2); 
	
	/*上报数据*/	
  if(mqtt_publish( sysCfg.parameter.data_socket, BASEINFOR_TOPIC , (char *)topicdata , topicdata[1]*256+topicdata[0]+4))
	{
	
	}
	
	if(braceletelectricity < BRACELET_WARN_ELECT)
	{
		beacon_warn_para[1] = braceletelectricity;
		ErrorLog(BRACELET,&topicdata[3],WARN,ELECT_WARN,1,beacon_warn_para); /*设备告警*/
	}
}


/*
*********************************************************************************************************
 * 函数名：app_palt_DealPlatinfo
 * 描述  ：处理平台信息,包括对对手环进行留言、设置信息机时间
 * 输入  ：TopicInfo,平台信息
 *			 : Len, 信息长度
 *			 : topic, 接收到消息的主题
 * 返回  : 无
*********************************************************************************************************
*/

static void app_palt_Datadeal(uint8_t *TopicInfo,uint16_t Len,char * topic)
{
	uint16_t crcdata; 
	app_plat_topic nState;
  							
  /*数据长度检查*/	
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
	
	/*CRC校验检查*/
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
			/*发送短消息*/
			case PLAT_RESPONSE_SEND_MESSAGE:
				if((TopicInfo[0]+TopicInfo[1]*256-29)>230)
				{		
					return;
				}
				#if APP_DEBUG
				printf("receive meseage\r\n");
				#endif
				
				/*保存消息数据到缓存中*/
				app_palt_WriteFifoData(TopicInfo,Len);
				break;
			
			/*网关设备升级*/
			case PLAT_RESPONSE_DEVUPDATE:
				IWDG_Feed();
			  bsp_ota_UpdataGateway(Len,TopicInfo);  
				break;
			
			 /*蓝牙手环升级*/
			case PLAT_RESPONSE_BRAUPDATE:
				IWDG_Feed();
			  bsp_ota_UpdataBracelet(Len,TopicInfo);  
				break;
			
			/*蓝牙设备升级开始指令*/
			case PLAT_RESPONSE_BLEUPDATE:
			  app_palt_UpdataBle(TopicInfo);  
				break;
			
			/*考勤信标管理*/
//	    case PLAT_RESPONSE_BEAMANAGEMENT:
//				app_palt_BeaconManagement(TopicInfo);
//				break;
			
			/*设置用户信息*/
			case PLAT_RESPONSE_USERINFOSET:
				app_palt_userinfoset( TopicInfo , Len );
				break;
			
			/*配置设备联网信息*/
			case PLAT_RESPONSE_DEVCFGSET:
				#if APP_DEBUG
				printf("receive user info.");
			  #endif
				app_palt_devcfgset( TopicInfo , Len );
				break;
			
			/*设备硬件重启*/
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
 * 函数名：app_palt_Receicedata
 * 描述  ：接收平台数据
 * 输入  ：plat_report_t,平台信息结构体
 * 返回  : 无
*********************************************************************************************************
*/
void app_palt_Receicedata(plat_report_t * report_t)
{
	uint8_t  len;
	uint8_t  dup;					//重复标志
	uint8_t  ack[30];			//回执
  uint8_t  retained;		//保留标志
  uint8_t  *payload_in; //负载内容
  
	uint16_t msgid;				//消息ID
	
	int qos;							//消息质量
	int payloadlen_in;		//负载长度
	MQTTString receivedTopic;
	
	/*解析消息内容*/
	MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &receivedTopic,
	&payload_in, &payloadlen_in, report_t->payload_data , PLAT_MAX_DATA_LEN );				

	/*消息回执 订阅的QOS=1需要回执*/
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
		else  /*第三方socket连接异常指示*/
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
 * 函数名：app_palt_Reportparse
 * 描述  ：解析平台数据包
 * 输入  ：plat_report_t,平台信息结构体
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
*********************************************************************************************************
*/
ErrorStatus app_palt_Reportparse(plat_report_t * report_t)
{
	uint16_t len = 0;
	uint8_t reg_status = 0;
	
	reg_status = getIR();               	/*读取中断标志寄存器*/
	setIR(reg_status);                  	/*回写清除中断标志*/
	reg_status = getSIR();              	/*读取端口中断标志寄存器*/	
	setSIR(reg_status);										/*回写清除端口中断标志寄存器*/
	
	//printf("reg_status：%02x\r\n",reg_status);
	
	/*主服务器端口事件*/
	if((reg_status & (1 << SOCK_TCPS)) == (1 << SOCK_TCPS))     
	{
		report_t->socket = SOCK_TCPS;
		reg_status = getSn_IR(SOCK_TCPS); 	/*读取Socket中断标志寄存器*/
		setSn_IR(SOCK_TCPS,reg_status);   	/*回写清除中断*/
		if(reg_status & Sn_IR_RECV)         /*Socket接收到数据,可以启动S_rx_process()函数*/
		{	
			len = getSn_RX_RSR(SOCK_TCPS);		/*读取W5500接收缓存区数据大小*/
			if(0 == len) 
			{
				return ERROR;
			}
			/*获取W5500接收缓存区数据*/
			report_t->evt_id = MQTTPacket_read(report_t->payload_data, PLAT_MAX_DATA_LEN , transport_getdata0);
			return SUCCESS;
		}
	}

	/*第三方服务器端口事件*/
	if((reg_status & (1 << SOCK_TCP)) == (1 << SOCK_TCP))     
	{
		report_t->socket = SOCK_TCP;
		reg_status = getSn_IR(SOCK_TCP);  	/*读取Socket中断标志寄存器*/
		setSn_IR(SOCK_TCP,reg_status);    	/*回写清除中断*/
		if(reg_status & Sn_IR_RECV)         /*Socket接收到数据,可以启动S_rx_process()函数*/
		{	
			len = getSn_RX_RSR(SOCK_TCP);			/*读取W5500接收缓存区数据大小*/
			if(0 == len) 
			{
				return ERROR;
			}
			/*获取W5500接收缓存区数据*/
			report_t->evt_id = MQTTPacket_read(report_t->payload_data, PLAT_MAX_DATA_LEN , transport_getdata1);
			return SUCCESS;
		}
	}
	
	/*BLE升级端口事件*/
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

