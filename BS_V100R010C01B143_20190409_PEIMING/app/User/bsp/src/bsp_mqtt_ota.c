#include "includes.h"

/*
*********************************************************************************************************
*	函 数 名: FLASH_PagesMask
*	功能说明: 根据总字节数计算要擦除的页数
*	形    参Size:总字节数	 
*	返 回 值: 页数
*********************************************************************************************************
*/
static uint32_t FLASH_PagesMask(uint32_t Size)
{
  uint32_t pagenumber = 0x0;
  uint32_t size = Size;

  if((size % 0x800) != 0)
  {
    pagenumber = (size / 0x800) + 1;
  }
  else
  {
    pagenumber = size / 0x800;
  }
  return pagenumber;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ota_ackResponse
*	功能说明: 升级响应
*	形    参: asktype，响应类型
*	        : serialnumber，数据流水号
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_ota_ackResponse( uint8_t asktype , uint8_t *serialnumber )
{
	uint16_t crcdata;
	uint8_t ackbuf[18]={0x0E,0x00,0x3A};
	
	ackbuf[2] = UPDATA;
	ackbuf[11] = asktype;
	
	memset(&ackbuf[3],0,2);                           /*网关设备ID低字节补齐*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*网关ID*/
	memcpy(&ackbuf[12],serialnumber,4);     				  /*数据包流水号*/
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(ackbuf,ackbuf[1]*256+ackbuf[0]+2);
	memcpy(&ackbuf[16],(uint8_t *)&crcdata,2);  
  mqtt_publish( SOCK_TCPS , UPDATABCD_TOPIC , (char *)ackbuf , ackbuf[1]*256+ackbuf[0]+4 );
}
/*
*********************************************************************************************************
*	函 数 名: bsp_ota_ackResponse
*	功能说明: 升级响应
*	形    参: asktype，响应类型
*	        : serialnumber，数据流水号
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_ble_ackResponse( uint8_t asktype , uint8_t *serialnumber, uint8_t *bracelet_mac)
{
	uint16_t crcdata;
	uint8_t i;
	uint8_t ackbuf[26]={0x16,0x00,0x9A};
	
	ackbuf[2] = BLE_UPDATA;
	
	memset(&ackbuf[3],0,2);                           /*网关设备ID低字节补齐*/
	memcpy(&ackbuf[5],sysCfg.parameter.client_mac,6); /*网关ID*/
	memset(&ackbuf[11],0,2); 
	memcpy(&ackbuf[13],bracelet_mac,6);               /*手环MAC*/
	ackbuf[19] = asktype;
	memcpy(&ackbuf[20],serialnumber,4);     				  /*数据包流水号*/
	
	/*CRC校验*/
	crcdata=app_plat_usMBCRC16(ackbuf,ackbuf[1]*256+ackbuf[0]+2);
	memcpy(&ackbuf[24],(uint8_t *)&crcdata,2); 

  mqtt_publish( SOCK_TCPS , UPDATABLE_TOPIC , (char *)ackbuf , ackbuf[1]*256+ackbuf[0]+4 );
}
/*
*********************************************************************************************************
*	函 数 名: mqtt_firmware
*	功能说明: 由mqtt协议下载固件到备份区
*	形    参buf：mqtt接收数据地址
            updatetopic：升级主题
            messgelen：负载消息长度
            payload:负载内容地址
*	返 回 值:
*********************************************************************************************************
*/
void bsp_ota_UpdataGateway(uint16_t messgelen,uint8_t* payload)
{
	uint8_t count=0;											//计数第几包 

  uint16_t i;
	uint16_t data;                        //接收数据
	uint16_t status;                      //Flash状态		

	uint32_t n_pages=0;                   //擦除的页
	uint32_t n_erased=0;
  uint32_t firmwaretype;
	
	static uint16_t  lastpacknum = 0;        //上一包固件序号
	static uint16_t  firmwarepacknum = 0;      //固件总包数
	
	static uint32_t rxlen=0;							//接收长度	
	static uint32_t content_len=0;				//总字节
	static uint32_t flashdest=BACKUPADRR;	//备份区地址
		
	/*包序号*/
	count = payload[3] + payload[4]*256;  
	
	uint8_t device_info[4] = {GATEWAY,STM32F103ZET6,TRANS_WW5500,NOSRAM};
	uint8_t ask[4] = {ACK_DEVTYPE_ERROR,ACK_MCUTYPE_ERROR,ACK_TRSTYPE_ERROR,ACK_SRATYPE_ERROR};
	
	if(lastpacknum == count)  /*重复固件包*/
	{
		bsp_ota_ackResponse( ACK_OK , &payload[messgelen-6] );	
    return;		
	}
	
	if(count == 1)       /*第一包数据*/
	{ 		
		content_len = 0;   /*清零总长度*/
		for(i=0;i<4;i++)   /*获取固件长度*/
		{
			content_len |= (payload[7+i] << (8 * i));
		}		
					
		/*固件长度错误*/
		if(BACKUPADRR + content_len - FIRM_INFO_LEN > FLAGADRR)
		{
			#if APP_DEBUG
			printf("firmware len error.\r\n");	 
			#endif
			bsp_ota_ackResponse( ACK_LEN_ERROR , &payload[messgelen-6] );
			return;
		}
		
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		n_pages = FLASH_PagesMask(content_len - FIRM_INFO_LEN);    /*计算要擦除的页*/
		for(n_erased = 0; n_erased < n_pages; n_erased++)      		 /*擦除页*/
		{			
			status=FLASH_ErasePage(BACKUPADRR + 0x800 * n_erased);
			if(status != 4)       /*擦除失败*/
			{
				#if APP_DEBUG
				printf("eraser flash fail\r\n");
				#endif
				bsp_ota_ackResponse( ACK_ERASE_ERROR , &payload[messgelen-6] );
				return;
			}
		}				
		
		rxlen = 0;                   /*接收计数清零*/
		lastpacknum=count;           /*上一包固件序号*/
		flashdest=BACKUPADRR;        /*写入开始地址*/
		firmwarepacknum=payload[5]+payload[6]*256; /*总包数*/
		
		#if APP_DEBUG
		printf("firmware_len:%d,total package num:%d\r\n",content_len - FIRM_INFO_LEN,firmwarepacknum);     /*固件总长度*/
		printf("eraser flash Sucessful\r\n");
		#endif
		
		bsp_ota_ackResponse( ACK_OK , &payload[messgelen-6] );
	}
	else if(count == 2)      
	{
		/*设备信息检查*/
		for(uint8_t i = 0; i < 4;i++)
		{
			if(payload[5+i] != device_info[i])
			{
				bsp_ota_ackResponse( ask[i] , &payload[messgelen-6] );
				#if APP_DEBUG
				printf("firmware type error code:%d\r\n",i);
				#endif
				return;
			}
		}
		
		if(payload[12] < SOFTVERSION)
		{                      
			bsp_ota_ackResponse( ACK_VERTYPE_ERROR , &payload[messgelen-6] );
			#if APP_DEBUG
			printf("firmware version error\r\n");
			#endif
			return;			
		}
		
		memcpy((uint8_t *)&firmwaretype,&payload[13],4);
		if((firmwaretype & 0x2FFE0000) != 0x20000000)
		{
			#if APP_DEBUG
			printf("firmware type error.\r\n");	 
			#endif
			bsp_ota_ackResponse( ACK_FIRTYPE_ERROR , &payload[messgelen-6] ); /*固件类型错误*/
			return;			
		}
		
		/*数据写入flash备份区*/
		for(i = 0;i < messgelen - 11 - FIRM_INFO_LEN ; i += 2)
		{
			if(flashdest > FLAGADRR)        /*判断是否写入越界*/
			{
				flashdest = BACKUPADRR;       /*写入开始地址*/
				#if APP_DEBUG
				printf("write firmware over backupadrr.\r\n");
				#endif 
				bsp_ota_ackResponse( ACK_ADDROV_ERROR , &payload[messgelen-6] );
				return;            
			}
			data = payload[i+6+FIRM_INFO_LEN];	
			data = (data<<8) + payload[i+5+FIRM_INFO_LEN];
			status = FLASH_ProgramHalfWord(flashdest, data);									      
			flashdest += 2;	 
			rxlen += 2;
			if(status != 4)        /*写入flash失败*/
			{
				#if APP_DEBUG
				printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*上一包固件序号*/		
		
		#if APP_DEBUG
		printf("download %6d bytes ok\r\n",rxlen);	 
		#endif		
		bsp_ota_ackResponse( ACK_OK , &payload[messgelen-6] );
	}
	else if(count > 2)       
	{
		/*数据写入flash备份区*/
		for(i=0;i<messgelen-11;i+=2)
		{
			if(flashdest > FLAGADRR)        /*判断是否写入越界*/
			{
				flashdest = BACKUPADRR;         /*写入开始地址*/
				#if APP_DEBUG
				printf("write firmware over backupadrr.\r\n");
				#endif
				bsp_ota_ackResponse( ACK_ADDROV_ERROR , &payload[messgelen-6] );
				return;            
			}
			data = payload[i+6];	
			data = (data<<8) + payload[i+5];
			status = FLASH_ProgramHalfWord(flashdest, data);									      
			flashdest += 2;	 
			rxlen += 2;
			if(status != 4)        /*写入flash失败*/
			{
				#if APP_DEBUG
				printf("write flash fail\r\n");
				#endif
				
				bsp_ota_ackResponse( ACK_WRITE_ERROR , &payload[messgelen-6] );
				return;
			}
		} 
		
		lastpacknum = count;          /*上一包固件序号*/		
		
		#if APP_DEBUG
		printf("download %6d bytes ok\r\n",rxlen);	 
		#endif
		if((rxlen == content_len - FIRM_INFO_LEN) && ((count-1) == firmwarepacknum))   	  /*接收数据完成*/
		{				
			bsp_ota_ackResponse( ACK_OK , &payload[messgelen-6] );
			
			/*写入文件长度*/									 
			if(!(bsp_WriteCpuFlash(FLAGADRR,(uint8_t *)&rxlen,4)))
			{
				#if APP_DEBUG
				printf("\r\nReady t o reboot.......\r\n");
				#endif
				/*断开Mqtt连接*/
				mqtt_disconnect();
				vTaskDelay(5000);
				__set_FAULTMASK(1);
				NVIC_SystemReset();  /*重启系统*/
			}
			else	/*写入标志位失败*/
			{
				#if APP_DEBUG
				printf("write flag fail\r\n");
        #endif			  
				bsp_ota_ackResponse( ACK_FLAG_ERROR , &payload[messgelen-6] );
				return;
			}
		}			
    else
		{
			bsp_ota_ackResponse( ACK_OK , &payload[messgelen-6] );
		}			
	}		 
}
/*
*********************************************************************************************************
*	函 数 名: app_nrf_SendBleCmd
*	功能说明: 蓝牙手环空升初始命令
*	形    参mac：目标手环mac
            len: 固件长度
						messge：空升初始化信息
*	返 回 值:
*********************************************************************************************************
*/
static void app_nrf_SendBleCmd(uint8_t *mac,uint8_t *len,uint8_t *messge)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t Sendbuf[31]={0x5A,0x49,0x00,0x19};

	Sendbuf[1] = OTA_CMD;         /*消息ID*/
  Sendbuf[4]=0x01;
	memcpy(&Sendbuf[5],mac,6);
	memcpy(&Sendbuf[11],len,4);
	memcpy(&Sendbuf[15],messge,14);
	check_results=Sendbuf[1];
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[i]=check_results;           //校验值
	Sendbuf[i+1]=0xca;                  //结束符

	comSendBuf( NRF_PORT , Sendbuf ,31 ); //发送数据包
}
/*
*********************************************************************************************************
*	函 数 名: app_nrf_SendBleData
*	功能说明: 蓝牙手环空升数据
*	形    参data：空升数据
*	返 回 值:
*********************************************************************************************************
*/
static void app_nrf_SendBleData(uint8_t *mac,uint8_t *data,uint16_t lenth)
{
	uint16_t i=0;
	uint8_t check_results=0;
	uint8_t Sendbuf[1013]={0x5A,0x49,0x03,0xEF};

	Sendbuf[1] = OTA_CMD;         /*消息ID*/
	Sendbuf[4]=0x02;
	memcpy(&Sendbuf[5],mac,6);
  memcpy(&Sendbuf[11],data,lenth);
	Sendbuf[2]=(lenth+7)/256;
	Sendbuf[3]=(lenth+7)%256;
	check_results=Sendbuf[1];
	for(i=2;i<(lenth+7+2+3-1);i++)   //CMD字段到数据字段进行校验
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[i]=check_results;           //校验值
	Sendbuf[i+1]=0xca;                  //结束符

	comSendBuf( NRF_PORT , Sendbuf ,lenth+13 );       //发送数据包
}

/*
*********************************************************************************************************
*	函 数 名: bsp_ota_UpdataBracelet
*	功能说明: 由mqtt协议转发蓝牙手环固件
*	形    参messgelen：负载消息长度
            payload:负载内容地址
*	返 回 值:
*********************************************************************************************************
*/
void bsp_ota_UpdataBracelet(uint16_t messgelen,uint8_t* payload)
{
	uint16_t count=0;											//计数第几包 
  uint16_t i,num = 0;

	uint8_t  byte;
  uint8_t  recbleacklen = 0;  
	uint8_t  readbuf[MAX_BLE_RESPONSE_LEN] = {0};

	uint32_t serialnumber;   /*流水号*/

	static uint8_t buf[6] = {0};
	static uint8_t updata_flag = 0;
  static uint8_t upd_bra[6] = {0};      //目标手环MAC
	static uint8_t new_bra[6] = {0};      //目标手环MAC
	static uint8_t firmwarelen[4] = {0}; 
	
	static uint16_t firmwarepacknum;  
	
	static uint32_t rxlen=0;							//接收长度	
	static uint32_t content_len=0;				//有效总字节
	
	/*包序号*/
	count = payload[3] + payload[4]*256;  

	if(count == 1)
	{
		content_len = 0;  																			   /*清零总长度*/
		memcpy((uint8_t *)&serialnumber,&payload[messgelen-6],4);  /*当前数据包流水号*/
		
		memcpy(buf,&payload[7],6);      									 /*当前升级手环MAC*/
		memcpy(firmwarelen,&payload[15],4);    /*手环固件总长度*/
		
		for(i = 0; i < 4; i++)  /*获取传输字节总长度*/
		{
			content_len |= (payload[15+i] << (8 * i));
		}		
		
		firmwarepacknum = payload[5] + payload[6]*256; /*总包数*/				
		rxlen = 0;     																 /*接收计数清零*/

		for(i = 0; i < 6; i++)
		{
			new_bra[i] = buf[5-i];
		}
		
		#if APP_DEBUG
		printf("firmware_len:%d,total package num:%d\r\n",content_len ,firmwarepacknum);     /*固件总长度*/
		printf("updata bracelet:");
		for(i = 0; i < 6; i++)
		{
			printf("%02X ",new_bra[i]);
		}
		printf("\r\n");
		#endif		
		
		if(updata_flag == 1)
		{
			#if APP_DEBUG
			printf("another bralecet data count:%d\r\n",count);
			for(i = 0; i < 6; i++)
			{
				printf("%02X ",new_bra[i]);
			}
			printf("\r\n");
			#endif		
			bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
		  return;
		}
				
		updata_flag = 1;
		memcpy(upd_bra,new_bra,6);
		bsp_ble_ackResponse( OTA_STA ,(uint8_t *)&serialnumber,upd_bra);	
	}
	else if(count == 2)      
	{ 			
		memcpy(buf,&payload[5],6);                                    /*当前升级手环MAC*/
		memcpy((uint8_t *)&serialnumber,&payload[messgelen-6],4);     /*当前数据包流水号*/
		
		for(i = 0; i < 6; i++)
		{
			new_bra[i] = buf[5-i];
		}
//		if(0 != memcmp(upd_bra,new_bra,6))
//		{
//			#if APP_DEBUG
//			printf("another bralecet data count:%d\r\n",count);
//			for(i = 0; i < 6; i++)
//			{
//				printf("%02X ",new_bra[i]);
//			}
//			printf("\r\n");
//			#endif		
//			bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
//		  return;
//		}
		
		comClearTxFifo(NRF_PORT);
    app_nrf_SendBleCmd(buf,firmwarelen,&payload[13]);	  /*发送密钥和固件长度*/
		
		#if APP_DEBUG
		printf("wait connect bralecet...\r\n");
    #endif
		
//		num = 0;		
//    while(1) 
//	  {	
//			num++;
//			if(num > 60)
//			{
//				#if APP_DEBUG
//				printf("connect bralecet outtime...\r\n");	
//				#endif	
				
//				bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
//				mqtt_disconnect();
//				vTaskDelay(2000);
//				__set_FAULTMASK(1);
//				NVIC_SystemReset();  /*重启系统*/
//				break;
//			}
   
    for(i = 0; i < 60; i++)
    {		
			IWDG_Feed();
			vTaskDelay(1000);

			memset(readbuf,0x00,sizeof(readbuf));
			if(SUCCESS == app_nrf_ReadData(readbuf) && readbuf[1] == 0x49)
			{
				if(readbuf[11] == 0x01)
				{
					#if APP_DEBUG
					printf("connect bralecet.\r\n");	
					#endif	
					bsp_ble_ackResponse( KEY_OK ,(uint8_t *)&serialnumber,&readbuf[5]);	
					return;
				}
				
				break;
//				else
//				{
//					#if APP_DEBUG
//					printf("connect bralecet fail...\r\n");	
//					#endif	
//					bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
//				}
//				break;
			}
		}
		
		bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
	}
	else if(count > 2)       
	{			
		memcpy(buf,&payload[5],6);                                    /*当前升级手环MAC*/
		memcpy((uint8_t *)&serialnumber,&payload[messgelen-6],4);     /*当前数据包流水号*/
		
		for(i = 0; i < 6; i++)
		{
			new_bra[i] = buf[5-i];
		}
		
//		if(0 != memcmp(upd_bra,new_bra,6))
//		{
//			#if APP_DEBUG
//			printf("another bralecet data count:%d\r\n",count);
//			for(i = 0; i < 6; i++)
//			{
//				printf("%02X ",new_bra[i]);
//			}
//			printf("\r\n");
//			#endif		
//			bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);	
//		  return;
//		}
		
		rxlen += (messgelen-19);
		#if APP_DEBUG
		printf("receive %6d bytes ok\r\n",rxlen);	 
		#endif
		
		while(comGetChar(NRF_PORT,&byte)); 
		app_nrf_SendBleData(buf,&payload[13],messgelen-19);
		
//		num = 0;	
//		while(1) 
//	  {	
//			num++;
//			if(num > 80)
//			{
		 for(i = 0; i < 80; i++)
     {
//				#if APP_DEBUG
//				printf("data fail...\r\n");	
//				#endif
//				bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,upd_bra);	
//				vTaskDelay(1000);
//				mqtt_disconnect();
//				vTaskDelay(1000);
//				__set_FAULTMASK(1);
//				NVIC_SystemReset();  /*重启系统*/
//				return;
//			}
			
			vTaskDelay(200);
			memset(readbuf,0x00,sizeof(readbuf));
			if((SUCCESS==app_nrf_ReadData(readbuf)) && (readbuf[1]==0x49))
			{
				bsp_ble_ackResponse( ACK_OK ,(uint8_t *)&serialnumber,&readbuf[5]);	
				
				//printf("rxlen:%d content_len:%d count:%d firmwarepacknum:%d\r\n",rxlen,content_len,count,firmwarepacknum);
				
				if((rxlen == content_len) && ((count) == firmwarepacknum))   	  /*接收数据完成*/
				{	
					mqtt_disconnect();	
					
					#if APP_DEBUG
					printf("Send ble data complete.\r\n");
					#endif	
				}	
				
				return;
			}
		}	

    bsp_ble_ackResponse( ACK_FAIL ,(uint8_t *)&serialnumber,new_bra);			
	}		 
}

