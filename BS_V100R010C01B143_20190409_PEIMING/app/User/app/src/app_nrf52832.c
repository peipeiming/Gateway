#include "includes.h"

static uint8_t Bracelet[BRACELET_COUNT][BRACELET_RECORD_LEN];

/*
*********************************************************************************************************
 * ��������app_nrf_ReadData
 * ����  ��nrf�������ݽ���
 * ����  ����
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
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
	if(i == 50) 		  /*���޴β���*/    
	{
		return ERROR;   
	}
	
	recebuf[0] = 0x5a;
	for(i = 1; i < 4; i++)  /*��������ͷ*/
	{
		if(0 == comGetChar(NRF_PORT,&recebuf[i]))
		{
			return ERROR; 
		}
	}
 
	len = recebuf[2]*256 + recebuf[3];
	if(len + 6 > MAX_BLE_RESPONSE_LEN)
	{ 			
		return ERROR;    		/*��ֹ�������Խ��*/
	}
	
	for(i = 0; i < len + 2; i++)  /*��Ч����*/
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

  /*������*/	
	if(check_results == recebuf[4+len])  
	{
		return SUCCESS;
	}

	return ERROR;  	
}

/*
*********************************************************************************************************
 * ��������app_nrf_AttendanceDetermine
 * ����  �������ж��㷨
 * ����  ��
 *       : station,��վ��ַ
 *       : location,λ������
 *       : bracelet,�ֻ�
 *       : sramaddr,�ֻ���ʷ����λ��
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
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
//					/*���ſ���*/
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
//					/*���ſ���*/
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
 * ��������app_plat_IsLeapYear
 * ����  ���ж��Ƿ�Ϊ����
 * ����  ��year,��
 * ����  : 1,�� 0,����
*********************************************************************************************************
*/
static uint8_t app_nrf_IsLeapYear(uint16_t year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}

/*
*********************************************************************************************************
 * ��������app_plat_TimeTosec
 * ����  ���ж��Ƿ�Ϊ����
 * ����  ��syear,smon,sday,hour,min,min,sec
 * ����  : seccount,
*********************************************************************************************************
*/
static uint32_t app_nrf_TimeTosec(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;

	uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//��������ݵ��������
	{
		if(1 == app_nrf_IsLeapYear(t))seccount+=31622400;//�����������
		else seccount+=31536000;			  //ƽ���������
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //��ǰ���·ݵ����������
	{
		seccount+=(uint32_t)mon_table[t]*86400;//�·����������
		if(app_nrf_IsLeapYear(syear)&&t==1)seccount+=86400;//����2�·�����һ���������	   
	}
	
	seccount+=(uint32_t)(sday-1)*86400;//��ǰ�����ڵ���������� 
	seccount+=(uint32_t)hour*3600;//Сʱ������
  seccount+=(uint32_t)min*60;	 //����������
	seccount+=sec;//�������Ӽ���ȥ
  
	return seccount;	    
}

/*
*********************************************************************************************************
 * ��������app_nrf_AddStation
 * ����  �������µ�������վ
 * ����  ��
 *       ��Station,��ӵ�485��վ��ַ
 * ����  : ��
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
	
	/*��ӡ�豸��Ϣ*/	
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
		
	/*������Ӧ����*/
	memcpy(&ackdata[4],Station,8);	
	check_results = ackdata[1];   
	for(uint8_t i = 2; i < (ackdata[3] + 4); i++)   //CMD�ֶε������ֶν���У��
	{
		check_results ^= ackdata[i];
	}
	ackdata[12] = check_results;                    //У��ֵ
	comSendBuf( NRF_PORT , ackdata , 14 );          //����ȷ�����ݰ�
	
	memcpy(&sysCfg.parameter.nrfstation,Station,8);  
  sysCfg.parameter.register_flag = REGISTER;  	
	 
	app_system_NetPublic();
}

/*
************************************************************************************************************
 * ��������app_nrf_GapInfoGet
 * ����  ����ȡ�㲥����
 * ����  ��
 *       : station,��վ��ַ
 * ����  : ��
 * ���䷽�� ����������-->������վ
��Э���ʽ������ʼ(1Byte)+��ϢID(1Byte)+��Ч��Ϣ����(2Byte)+Ҫ��ȡ�㲥�Ķ�ͷID(8Byte)+У��(1Byte)+����(1Byte)
 *   �ϼ�   : 14(Byte)
************************************************************************************************************
 */
void app_nrf_GetBleData(uint8_t *station)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t Sendbuf[14]={0x5A,0x41,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xCA};

	Sendbuf[1] = GETBLEDATA;         /*��ϢID*/
	memcpy(&Sendbuf[4],station,8);   /*��ͷ��ַ*/
	
	check_results=Sendbuf[1];
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD�ֶε������ֶν���У��
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //У��ֵ
	Sendbuf[i+1]=0xca;                  //������
	
	comSendBuf( NRF_PORT , Sendbuf , 14 );       //�������ݰ�
}

/*
************************************************************************************************************
 * ��������app_nrf_UpdataBle
 * ����  �������������ƿ�ʼָ��
 * ����  ��
 *       : station,��վ��ַ
 * ����  : ��
************************************************************************************************************
 */
void app_nrf_UpdataBle(uint8_t *station)
{
	uint8_t i = 0;
	uint8_t check_results = 0;
	uint8_t Sendbuf[14]={0x5A,0x46,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x49,0xCA};

	Sendbuf[1] = UPDATA_STATION;     /*��ϢID*/
	for(i = 0; i < 8; i++)
	{
		Sendbuf[4+i] = station[7-i];
	}
	
	check_results=Sendbuf[1];
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD�ֶε������ֶν���У��
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //У��ֵ
	Sendbuf[i+1]=0xca;                  //������
	
	#if 0
	printf("send ble device:");
	for(i = 0; i < 14; i++)
	{
		printf("%02x ",Sendbuf[i]);
	}
	printf("\r\n");
	#endif
	
	comSendBuf( NRF_PORT , Sendbuf , 14 );       //�������ݰ�
}

/*
*********************************************************************************************************
 * ��������app_nrf_ConnectBle
 * ����  ������Ŀ���ֻ�
 * ����  ��
 *       : station,��վ��ַ
 *       : bracelet,�ֻ�
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
 * ���䷽�� ����������-->������վ
��Э���ʽ������ʼ(1Byte)+��ϢID(1Byte)+��Ч��Ϣ����(2Byte)+��ͷID(8Byte)+�ֻ�MAC��ַ(6Byte)
+У��(1Byte)+����(1Byte)
 *   �ϼ�   : 20(Byte)
*********************************************************************************************************
 */
ErrorStatus app_nrf_ConnectBle(uint8_t *station,uint8_t *bracelet)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
	uint8_t Sendbuf[20]={0x5A,0x42,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	
	Sendbuf[1]=CONNECTBLE;              /*��ϢID*/
	 
	memcpy(&Sendbuf[4],station,8);      /*��վ��ַ*/
	memcpy(&Sendbuf[12],bracelet,6);    /*MAC��ַ*/
		        
  #if APP_DEBUG
	printf("connect to:");
	for(uint8_t i = 0; i < 6; i++)
	{
		printf("%02X ",bracelet[i]);
	}
	printf("\r\n");
  #endif  
		
	check_results=Sendbuf[1];   
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD�ֶε������ֶν���У��
		check_results^=Sendbuf[i];
	Sendbuf[i]=check_results;           //У��ֵ
	Sendbuf[i+1]=0xca;                  //������	
		
//	while(SUCCESS == app_nrf_ReadData(readbuf))  /*���ս��ջ���*/
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
 * ��������app_nrf_DisconnectBle
 * ����  ���Ͽ���������
 *       : station,��վ��ַ
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
 * ���䷽�� ����������-->������վ
��Э���ʽ������ʼ(1Byte)+��ϢID(1Byte)+��Ч��Ϣ����(2Byte)+��ͷID(8Byte)+У��(1Byte)+����(1Byte)
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
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD�ֶε������ֶν���У��
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[12]=check_results;          //У��ֵ
	Sendbuf[13]=0xca;                   //������	
		
	for(i=0;i<10;i++)
	{
		comSendBuf( NRF_PORT , Sendbuf , Sendbuf[2]*256+Sendbuf[3]+6 );  	 //��������
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
 * ��������app_nrf_LeaveMessage
 * ����  ���ֻ���Ϣ����
 * ����  ��
 *       : station,��վ��ַ
 *       ��message��������Ϣ����
 *       ��len��������Ϣ���ݳ���
 * ����  : 
 *       ��SUCCESS,�ɹ���ERROR,ʧ��
 * ���䷽�� ����������-->������վ
��Э���ʽ������ʼ(1Byte)+��ϢID(1Byte)+��Ч��Ϣ����(2Byte)+��ͷID(8Byte)+��Ϣ����(NByte)
+У��(1Byte)+����(1Byte)
*********************************************************************************************************
 */
ErrorStatus app_nrf_LeaveMessage(uint8_t *station,uint8_t *message,uint16_t len)
{
	uint8_t i=0;
	uint8_t check_results=0;
	uint8_t readbuf[MAX_BLE_RESPONSE_LEN] = {0};
	uint8_t Sendbuf[1024]={0x5A,0x43,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x20,0x05};
  
	Sendbuf[1]=SEND_MESSAGE;            /*��ϢID*/
	
  Sendbuf[2]=(len+8)/256;               
  Sendbuf[3]=(len+8)%256;

	memcpy(&Sendbuf[4],station,8);      /*��վ�豸��ַ*/
	memcpy(&Sendbuf[12],message,len);   /*��Ϣ*/
	
	check_results=Sendbuf[1];   
	for(i=2;i<(Sendbuf[3]+2+3-1);i++)   //CMD�ֶε������ֶν���У��
	{
		check_results^=Sendbuf[i];
	}
	Sendbuf[len+8+4]=check_results;     //У��ֵ
	Sendbuf[len+8+5]=0xca;              //������
//		
//	while(SUCCESS == app_nrf_ReadData(readbuf))  /*���ս��ջ���*/
//	{
//		IWDG_Feed();
//	}
	
	comSendBuf( NRF_PORT , Sendbuf , Sendbuf[2]*256+Sendbuf[3]+6 );        
	
	for(i=0;i<30;i++)                   //�ȴ���Ӧ
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
 * ��������app_nrf_ResetStation
 * ����  ����λ����
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_nrf_ResetStation(void)
{
	uint8_t reset_ble_data[14] = {0x5a,0x45,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4d,0xca};
	
	comSendBuf( NRF_PORT , reset_ble_data , sizeof(reset_ble_data) );       //��λ����
  comSendBuf( NRF_PORT , reset_ble_data , sizeof(reset_ble_data) );       //��λ����
}

/*
*********************************************************************************************************
 * ��������app_nrf_SetRssi
 * ����  �������ֻ�ɨ���ű����ֵ
 * ����  ����
 * ����  : ��
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
	
	comSendBuf( NRF_PORT , reqdata , sizeof(reqdata) );       //�������ݰ�
}
/*
*********************************************************************************************************
 * ��������app_nrf_GetNewStation
 * ����  ������Ƿ��µĻ�վ�豸����
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_nrf_GetNewStation(void)
{
	uint8_t reqdata[14] = {0x5a,0x44,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x4c,0xca};
	
	comSendBuf( NRF_PORT , reqdata , sizeof(reqdata) );       //�������ݰ�
}


/*
*********************************************************************************************************
 * ��������app_nrf_DealBleData
 * ����  ������nrf��������
 * ����  ����
 * ����  : ��
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
			/*�����㷨*/
			for(i=0; i < BraceletNumber; i++)
			{		
				/*�Ǹ��ֻ���¼*/
				if(0 == memcmp(Bracelet[i],&readbuf[12],6)) 
				{					
					/*�˶������ı��ж�*/
					if(0 != memcmp(&Bracelet[i][8],&readbuf[12+6+6+5],6))
					{
						memcpy((uint8_t *)&Bracelet[i][8],&readbuf[12+6+6+5],6);						
						app_plat_SportDataPublish(&readbuf[12],&readbuf[12+6+6+5]);
					}	
					
			
					/*˯�������ϱ�*/
					if(0 != memcmp(&Bracelet[i][18],&readbuf[12+6+1],6))
					{
						memcpy((uint8_t *)&Bracelet[i][18],&readbuf[12+6+1],6);						
						app_plat_SleepDataPublish(&readbuf[12],&readbuf[12+6+1]);   
					}
					
					/*�ж����ʲ����Ƿ�ı�*/
					if(Bracelet[i][14] != readbuf[12+6+6+5+6] && readbuf[12+6+6+5+6] != 0 )
					{
						Bracelet[i][14] = readbuf[12+6+6+5+6];										
						app_plat_HeartratePublish(&readbuf[12],readbuf[12+6+6+5+6]);
					}
					
					/*�ж�SOS״̬�Ƿ�ı�*/
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
							ErrorLog(BRACELET,bracelet,WARN,SOS_WARN,0,NULL); 	/*�豸�澯*/
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
							ErrorLog(BRACELET,bracelet,WARN,SOS_QUIT,0,NULL); 	/*�豸�澯*/
						}
					}
					
					/*���ڡ�λ�����ݸı��ϱ�*/
          if(Bracelet[i][6] != readbuf[12+6+6+1] || Bracelet[i][7] != readbuf[12+6+6+3]) 
					{			
						//app_nrf_AttendanceDetermine(&readbuf[4],&readbuf[12+6],&readbuf[12], i ); 	/*�����ж�*/
						app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
						
						Bracelet[i][6] = readbuf[12+6+6+1];
						Bracelet[i][7] = readbuf[12+6+6+3];	
						
						Bracelet[i][16] = nowsec % 100; 	 
					}
					else if( nowsec % 100 - Bracelet[i][16] > 10)  /*λ�ò�����ʱ�ϱ�*/
					{
						Bracelet[i][16] = nowsec % 100;
						app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
					}			
				
					/*�ֻ����������ı��ж�*/
					if(Bracelet[i][15] != readbuf[12+6+6+5+6+2] && readbuf[12+6+6+5+6+2] != 0) 
					{
						Bracelet[i][15] = readbuf[12+6+6+5+6+2];										
						app_plat_BraceletInfoPublish(&readbuf[12],readbuf[12+6+6+5+6+2]);
					}		
					
					break;  /*�˳�����*/
				}
			}
			
			/*û�и��ֻ���¼*/ 
			if(i == BraceletNumber)   
			{
				//printf("BraceletNumber:%d\r\n",BraceletNumber);
				BraceletNumber++; 							
				Bracelet[i][16] = nowsec % 100;      
				BraceletNumber %= BRACELET_COUNT;
				memcpy(Bracelet[i],&readbuf[12],6);  /*�洢�ֻ�MAC*/

				/*�洢���ϱ��˶�����*/
				memcpy((uint8_t *)&Bracelet[i][8],&readbuf[12+6+6+5],6);
				app_plat_SportDataPublish(&readbuf[12],&readbuf[12+6+6+5]); 
			
				/*�洢���ϱ�λ������*/
				Bracelet[i][6] = readbuf[12+6+6+1];
				Bracelet[i][7] = readbuf[12+6+6+3];	
				app_plat_LocationPublish(&readbuf[12],&readbuf[12+6],readbuf[12+6+6+5+6+3],&readbuf[4]);
								
				/*�ϱ�˯������*/
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
