#include "includes.h"

static uint8_t Bracelet[BRACELET_COUNT][BRACELET_RECORD_LEN];

/*
*********************************************************************************************************
 * 函数名：app_nrf_ReadData
 * 描述  ：nrf串口数据解析
 * 输入  ：无
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
*********************************************************************************************************
 */
ErrorStatus app_nrf_ReadData(uint8_t *recebuf)
{
	uint8_t i = 0;
	uint8_t len = 0;
	uint8_t recdata = 0;
	uint8_t check_results = 0; 
	
	while(1 == comGetChar(NRF_PORT,&recdata) && i++ < 50)
	{ 
		if(0x5a == recdata)
		{
			break;
		}
	}
	if(i == 50) 		  /*有限次查找*/    
	{
		return ERROR;   
	}
	
	recebuf[0] = 0x5a;
	for(i = 1; i < 4; i++)  /*接收数据头*/
	{
		if(0 == comGetChar(NRF_PORT,&recebuf[i]))
		{
			return ERROR; 
		}
	}
 
	len = recebuf[2]*256 + recebuf[3];
	if(len + 6 > MAX_BLE_RESPONSE_LEN)
	{ 			
		return ERROR;    		/*防止数组访问越界*/
	}
	
	for(i = 0; i < len + 2; i++)  /*有效数据*/
	{
		if(0 == comGetChar(NRF_PORT,&recebuf[4+i]))
		{
			return ERROR; 
		}
	}

	if(recebuf[len + 6 - 1] != 0xca)
	{
		return ERROR;  
	}
	
	check_results = recebuf[1];                                  
	for(i = 2; i < len + 4; i++)     	
	{
		check_results ^= recebuf[i];
	}

  /*检查检验*/	
	if(check_results == recebuf[4+len])  
	{
		return SUCCESS;
	}

	return ERROR;  	
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_AttendanceDetermine
 * 描述  ：考勤判断算法
 * 输入  ：
 *       : station,基站地址
 *       : location,位置数据
 *       : bracelet,手环
 *       : sramaddr,手环历史数据位置
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
*********************************************************************************************************
 */
//static void app_nrf_AttendanceDetermine(uint8_t *station , uint8_t *location , uint8_t *bracelet , uint8_t sramaddr)
//{
//	uint8_t i,j=0;
//	
//	for(i=0;i<sysCfg.parameter.beacount;i++)
//	{		
//		if(location[7] == (sysCfg.parameter.bealist[i][0]+sysCfg.parameter.bealist[i][1]*256))
//		{
//			for(j=0;i<sysCfg.parameter.beacount;i++)
//			{
//				if(location[9] == (sysCfg.parameter.bealist[i][2]+sysCfg.parameter.bealist[i][3]*256))
//				{	
//					/*进门考勤*/
//					#if APP_DEBUG
//					for(j=0;j<6;j++)
//					{
//						printf("%02x ",bracelet[j]);
//					}
//					printf("Attendance:in   ");
//					for(j=0;j<8;j++)
//					{
//						printf("%02x ",station[j]);
//					}
//					printf("\r\n");
//					#endif
//					app_plat_AttendancePublish( bracelet , location , station , 1 );
//					break;
//			  }
//			}
//			return;
//		}
//		else if(location[7] == (sysCfg.parameter.bealist[i][2]+sysCfg.parameter.bealist[i][3]*256))
//		{
//			for(i=0;i<sysCfg.parameter.beacount;i++)
//			{
//				if(location[9] == (sysCfg.parameter.bealist[i][0]+sysCfg.parameter.bealist[i][1]*256))
//				{	
//					/*进门考勤*/
//					#if APP_DEBUG
//					for(j=0;j<6;j++)
//					{
//						printf("%02x ",bracelet[j]);
//					}
//					printf("Attendance:out   ");
//					for(j=0;j<8;j++)
//					{
//						printf("%02x ",station[j]);
//					}
//					printf("\r\n");
//					#endif
//					app_plat_AttendancePublish( bracelet , location , station , 2 );
//					break;
//			  }
//			}
//			return;
//		}
//	}
//}


/*
*********************************************************************************************************
 * 函数名：app_plat_IsLeapYear
 * 描述  ：判断是否为闰年
 * 输入  ：year,年
 * 返回  : 1,是 0,不是
*********************************************************************************************************
*/
static uint8_t app_nrf_IsLeapYear(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

/*
*********************************************************************************************************
 * 函数名：app_plat_TimeTosec
 * 描述  ：判断是否为闰年
 * 输入  ：syear,smon,sday,hour,min,min,sec
 * 返回  : seccount,
*********************************************************************************************************
*/
static uint32_t app_nrf_TimeTosec(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;

	uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(1 == app_nrf_IsLeapYear(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(app_nrf_IsLeapYear(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	
	seccount+=(uint32_t)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(uint32_t)hour*3600;//小时秒钟数
  seccount+=(uint32_t)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去
  
	return seccount;	    
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_AddStation
 * 描述  ：增加新的蓝牙基站
 * 输入  ：
 *       ：Station,添加的485基站地址
 * 返回  : 无
*********************************************************************************************************
 */
static void app_nrf_NewStation(uint8_t *Station)
{
	uint8_t check_results = 0;
	uint8_t ackdata[14] = {0x5a,0x44,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0xca};

	if(sysCfg.parameter.register_flag == REGISTER)
	{
		return;
	}
	
	/*打印设备信息*/	
  #if 1	
	printf("0000");
	for(uint8_t i = 0; i < 6; i++)
	{
		printf("%02X",sysCfg.parameter.client_mac[i]);
	}
	
	printf("   ");
	for(uint8_t i = 0; i < 8; i++)
	{
		printf("%02X",Station[i]);
	}
	printf("\r\n");	
  #endif
		
	/*返回响应数据*/
	memcpy(&ackdata[4],Station,8);	
	check_results = ackdata[1];   
	for(uint8_t i = 2; i < (ackdata[3] + 4); i++)   //CMD字段到数据字段进行校验
	{
		check_results ^= ackdata[i];
	}
	ackdata[12] = check_results;                    //校验值
	comSendBuf( NRF_PORT , ackdata , 14 );          //发送确认数据包
	
	memcpy(&sysCfg.parameter.nrfstation,Station,8);  
  sysCfg.parameter.register_flag = REGISTER;  	
	 
	app_system_NetPublic();
}

/*
************************************************************************************************************
 * 函数名：app_nrf_GapInfoGet
 * 描述  ：获取广播数据
 * 输入  ：
 *       : station,基站地址
 * 返回  : 无
 * 传输方向 ：蓝牙网关-->蓝牙基站
【协议格式】：起始(1Byte)+消息ID(1Byte)+有效信息长度(2Byte)+要获取广播的读头ID(8Byte)+校验(1Byte)+结束(1Byte)
 *   合计   : 14(Byte)
************************************************************************************************************
 */
void app_nrf_GetBleData(uint8_t *station)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t Sendbuf[14]={0x5A,0x41,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xCA};

	Sendbuf[1] = GETBLEDATA;         /*消息ID*/
	memcpy(&Sendbuf[4],station,8);   /*读头地址*/
	
	check_results=Sendbuf[1];
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //校验值
	Sendbuf[i+1]=0xca;                  //结束符
	
	comSendBuf( NRF_PORT , Sendbuf , 14 );       //发送数据包
}

/*
************************************************************************************************************
 * 函数名：app_nrf_UpdataBle
 * 描述  ：升级蓝牙估计开始指令
 * 输入  ：
 *       : station,基站地址
 * 返回  : 无
************************************************************************************************************
 */
void app_nrf_UpdataBle(uint8_t *station)
{
	uint8_t i = 0;
	uint8_t check_results = 0;
	uint8_t Sendbuf[14]={0x5A,0x46,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xCA};

	Sendbuf[1] = UPDATA_STATION;     /*消息ID*/
	for(i = 0; i < 8; i++)
	{
		Sendbuf[4+i] = station[7-i];
	}
	
	check_results=Sendbuf[1];
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //校验值
	Sendbuf[i+1]=0xca;                  //结束符
	
	#if 0
	printf("send ble device:");
	for(i = 0; i < 14; i++)
	{
		printf("%02x ",Sendbuf[i]);
	}
	printf("\r\n");
	#endif
	
	comSendBuf( NRF_PORT , Sendbuf , 14 );       //发送数据包
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_ConnectBle
 * 描述  ：连接目标手环
 * 输入  ：
 *       : station,基站地址
 *       : bracelet,手环
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
 * 传输方向 ：蓝牙网关-->蓝牙基站
【协议格式】：起始(1Byte)+消息ID(1Byte)+有效信息长度(2Byte)+读头ID(8Byte)+手环MAC地址(6Byte)
+校验(1Byte)+结束(1Byte)
 *   合计   : 20(Byte)
*********************************************************************************************************
 */
ErrorStatus app_nrf_ConnectBle(uint8_t *station,uint8_t *bracelet)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
	uint8_t Sendbuf[20]={0x5A,0x42,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	Sendbuf[1]=CONNECTBLE;              /*消息ID*/
	 
	memcpy(&Sendbuf[4],station,8);      /*基站地址*/
	memcpy(&Sendbuf[12],bracelet,6);    /*MAC地址*/
		        
  #if APP_DEBUG
	printf("connect to:");
	for(uint8_t i = 0; i < 6; i++)
	{
		printf("%02X ",bracelet[i]);
	}
	printf("\r\n");
  #endif  
		
	check_results=Sendbuf[1];   
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //校验值
	Sendbuf[i+1]=0xca;                  //结束符	
		
//	while(SUCCESS == app_nrf_ReadData(readbuf))  /*读空接收缓存*/
//	{
//		IWDG_Feed();
//	}
	
	comSendBuf( NRF_PORT , Sendbuf , sizeof(Sendbuf) );  	
  
	for(i = 0; i < 60; i++)
	{
		vTaskDelay(1000);
		IWDG_Feed();
		for(uint8_t j = 0; j < 50; j++)
		{
			memset(readbuf,0x00,sizeof(readbuf));
			if(SUCCESS == app_nrf_ReadData(readbuf) && readbuf[1] == 0x42)
			{ 
				if(0 == memcmp(&readbuf[12],&Sendbuf[12],6))
				{
					return SUCCESS;
				}
			}
		}
  }	
  
	#if APP_DEBUG
	printf("app_nrf_LinkTargetBracelet:no responsed!\r\n");
	#endif
	return ERROR;
}

/*
**************************************************************************************************
 * 函数名：app_nrf_DisconnectBle
 * 描述  ：断开蓝牙连接
 *       : station,基站地址
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
 * 传输方向 ：蓝牙网关-->蓝牙基站
【协议格式】：起始(1Byte)+消息ID(1Byte)+有效信息长度(2Byte)+读头ID(8Byte)+校验(1Byte)+结束(1Byte)
**************************************************************************************************
*/
ErrorStatus app_nrf_DisconnectBle(uint8_t *station)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
	uint8_t Sendbuf[14]={0x5A,0x40,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	memcpy(&Sendbuf[4],station,8);
	
	Sendbuf[1]=DISCONNECTBLE;
	check_results=Sendbuf[1];   
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[12]=check_results;          //校验值
	Sendbuf[13]=0xca;                   //结束符	
		
	for(i=0;i<10;i++)
	{
		comSendBuf( NRF_PORT , Sendbuf , Sendbuf[2]*256+Sendbuf[3]+6 );  	 //发送数据
		vTaskDelay(500);
		IWDG_Feed();
		memset(readbuf,0x00,sizeof(readbuf));
		for(uint8_t j = 0; j < 50; j++)
		{
			if(SUCCESS == app_nrf_ReadData(readbuf) && readbuf[1] == 0x40)
			{ 
				return SUCCESS;
			}
		}
	}
	return ERROR;
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_LeaveMessage
 * 描述  ：手环信息留言
 * 输入  ：
 *       : station,基站地址
 *       ：message，留言信息数据
 *       ：len，留言信息数据长度
 * 返回  : 
 *       ：SUCCESS,成功；ERROR,失败
 * 传输方向 ：蓝牙网关-->蓝牙基站
【协议格式】：起始(1Byte)+消息ID(1Byte)+有效信息长度(2Byte)+读头ID(8Byte)+消息内容(NByte)
+校验(1Byte)+结束(1Byte)
*********************************************************************************************************
 */
ErrorStatus app_nrf_LeaveMessage(uint8_t *station,uint8_t *message,uint16_t len)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
	uint8_t Sendbuf[1024]={0x5A,0x43,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x20,0x05};
  
	Sendbuf[1]=SEND_MESSAGE;            /*消息ID*/
	
  Sendbuf[2]=(len+8)/256;               
  Sendbuf[3]=(len+8)%256;

	memcpy(&Sendbuf[4],station,8);      /*基站设备地址*/
	memcpy(&Sendbuf[12],message,len);   /*消息*/
	
	check_results=Sendbuf[1];   
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[len+8+4]=check_results;     //校验值
	Sendbuf[len+8+5]=0xca;              //结束符
//		
//	while(SUCCESS == app_nrf_ReadData(readbuf))  /*读空接收缓存*/
//	{
//		IWDG_Feed();
//	}
	
	comSendBuf( NRF_PORT , Sendbuf , Sendbuf[2]*256+Sendbuf[3]+6 );        
	
	for(i=0;i<30;i++)                   //等待响应
	{
		IWDG_Feed();
		vTaskDelay(1000);
		
		for(uint8_t j = 0; j < 50; j++)
		{
			memset(readbuf,0x00,sizeof(readbuf));
			if(SUCCESS == app_nrf_ReadData(readbuf) && readbuf[1] == 0x43)
			{ 
				return SUCCESS;
			}
		}
  }		
	#if APP_DEBUG
	printf("Leave a message fail_2!\r\n");
  #endif
	return ERROR;
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_ResetStation
 * 描述  ：复位蓝牙
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_nrf_ResetStation(void)
{
	uint8_t reset_ble_data[14] = {0x5a,0x45,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4d,0xca};
	
	comSendBuf( NRF_PORT , reset_ble_data , sizeof(reset_ble_data) );       //复位蓝牙
  comSendBuf( NRF_PORT , reset_ble_data , sizeof(reset_ble_data) );       //复位蓝牙
}

/*
*********************************************************************************************************
 * 函数名：app_nrf_SetRssi
 * 描述  ：设置手环扫描信标过滤值
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_nrf_SetRssi(uint8_t rssi)
{ 
	uint8_t i = 0;
	uint8_t check_results = 0;
	uint8_t reqdata[14] = {0x5a,0x48,0x00,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0xca};
	
	check_results = reqdata[1];   
	reqdata[11] = rssi; 
	for(i = 2; i< (reqdata[3]+2+3-1); i++)  
	{
		check_results ^= reqdata[i];
	}
	reqdata[i] = check_results;           
	reqdata[i+1] = 0xca;             
	
	comSendBuf( NRF_PORT , reqdata , sizeof(reqdata) );       //发送数据包
}
/*
*********************************************************************************************************
 * 函数名：app_nrf_GetNewStation
 * 描述  ：检查是否新的基站设备接入
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_nrf_GetNewStation(void)
{
	uint8_t reqdata[14] = {0x5a,0x44,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0xca};
	
	comSendBuf( NRF_PORT , reqdata , sizeof(reqdata) );       //发送数据包
}


/*
*********************************************************************************************************
 * 函数名：app_nrf_DealBleData
 * 描述  ：处理nrf串口数据
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
*/
void app_nrf_DealBleData(void)
{		
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
		
	uint16_t i = 0;
	
	uint32_t nowsec = 0;

	DateTime nowtime;
	static uint32_t BraceletNumber = 0;
	
	get_ntp_time(&nowtime);
	
	if((nowtime.time.year[0]+nowtime.time.year[1]*256-2000)<0
		||(nowtime.time.year[0]+nowtime.time.year[1]*256-2000)>100)
	{
		return;
	}

	#ifdef MCBR03
	if(0 == nowtime.time.minute % 40 && 0 == nowtime.time.second % 59)
	{
		app_plat_SHTDataPublish();
		nowtime.time.second += 2;
	}
  #endif
	
	nowsec = app_nrf_TimeTosec(nowtime.time.year[0]+nowtime.time.year[1]*256,nowtime.time.month,
	nowtime.time.day,nowtime.time.hour,nowtime.time.minute,nowtime.time.second);
	
	while(SUCCESS == app_nrf_ReadData(readbuf))   
	{	
		if(readbuf[1] == GETBLEDATA)
		{										
			/*滤重算法*/
			for(i=0; i < BraceletNumber; i++)
			{		
				/*是该手环记录*/
				if(0 == memcmp(Bracelet[i],&readbuf[12],6)) 
				{					
					/*运动参数改变判断*/
					if(0 != memcmp(&Bracelet[i][8],&readbuf[12+6+6+5],6))
					{
						memcpy((uint8_t *)&Bracelet[i][8],&readbuf[12+6+6+5],6);						
						app_plat_SportDataPublish(&readbuf[12],&readbuf[12+6+6+5]);
					}	
					
			
					/*睡眠数据上报*/
					if(0 != memcmp(&Bracelet[i][18],&readbuf[12+6+1],6))
					{
						memcpy((uint8_t *)&Bracelet[i][18],&readbuf[12+6+1],6);						
						app_plat_SleepDataPublish(&readbuf[12],&readbuf[12+6+1]);   
					}
					
					/*判断心率参数是否改变*/
					if(Bracelet[i][14] != readbuf[12+6+6+5+6] && readbuf[12+6+6+5+6] != 0 )
					{
						Bracelet[i][14] = readbuf[12+6+6+5+6];										
						app_plat_HeartratePublish(&readbuf[12],readbuf[12+6+6+5+6]);
					}
					
					/*判断SOS状态是否改变*/
					if(Bracelet[i][17] != readbuf[12+6+6+5+6+1])
					{
						uint8_t bracelet[8] = {0};
						
						Bracelet[i][17] = readbuf[12+6+6+5+6+1];		
						if(0x01==Bracelet[i][17])
						{
//							printf("SOS:");
//							for(uint8_t k = 0; k < 6; k++)
//						  {
//								printf("%02X",readbuf[12+k]);
//							}
//							printf("\r\n");
							memcpy(bracelet,&readbuf[12],6);
							ErrorLog(BRACELET,bracelet,WARN,SOS_WARN,0,NULL); 	/*设备告警*/
						}
						else
						{
//							printf("SOS QUIT:");
//							for(uint8_t k = 0; k < 6; k++)
//						  {
//								printf("%02X",readbuf[12+k]);
//							}
//							printf("\r\n");
							
							memcpy(bracelet,&readbuf[12],6);
							ErrorLog(BRACELET,bracelet,WARN,SOS_QUIT,0,NULL); 	/*设备告警*/
						}
					}
					
					/*考勤、位置数据改变上报*/
          if(Bracelet[i][6] != readbuf[12+6+6+1] || Bracelet[i][7] != readbuf[12+6+6+3]) 
					{			
						//app_nrf_AttendanceDetermine(&readbuf[4],&readbuf[12+6],&readbuf[12], i ); 	/*考勤判断*/
						app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
						
						Bracelet[i][6] = readbuf[12+6+6+1];
						Bracelet[i][7] = readbuf[12+6+6+3];	
						
						Bracelet[i][16] = nowsec % 100; 	 
					}
					else if( nowsec % 100 - Bracelet[i][16] > 10)  /*位置参数定时上报*/
					{
						Bracelet[i][16] = nowsec % 100;
						app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
					}			
				
					/*手环电量参数改变判断*/
					if(Bracelet[i][15] != readbuf[12+6+6+5+6+2] && readbuf[12+6+6+5+6+2] != 0) 
					{
						Bracelet[i][15] = readbuf[12+6+6+5+6+2];										
						app_plat_BraceletInfoPublish(&readbuf[12],readbuf[12+6+6+5+6+2]);
					}		
					
					break;  /*退出查找*/
				}
			}
			
			/*没有该手环记录*/ 
			if(i == BraceletNumber)   
			{
				//printf("BraceletNumber:%d\r\n",BraceletNumber);
				BraceletNumber++; 							
				Bracelet[i][16] = nowsec % 100;      
				BraceletNumber %= BRACELET_COUNT;
				memcpy(Bracelet[i],&readbuf[12],6);  /*存储手环MAC*/

				/*存储、上报运动数据*/
				memcpy((uint8_t *)&Bracelet[i][8],&readbuf[12+6+6+5],6);
				app_plat_SportDataPublish(&readbuf[12],&readbuf[12+6+6+5]); 
			
				/*存储、上报位置数据*/
				Bracelet[i][6] = readbuf[12+6+6+1];
				Bracelet[i][7] = readbuf[12+6+6+3];	
				app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
								
				/*上报睡眠数据*/
				app_plat_SleepDataPublish(&readbuf[12],&readbuf[12+6+1]);   
				memcpy((uint8_t *)&Bracelet[i][18],&readbuf[12+6+1],6);					
			} 
	  }
		else if(readbuf[1] == GET_STATION)
		{			
			app_nrf_NewStation( &readbuf[4] );  
	  }
		memset(readbuf,0x00,sizeof(readbuf));
  }
}
