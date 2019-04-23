#include "includes.h"

/*
*********************************************************************************************************
 * ��������app_system_NetLedToggle
 * ����  ����������״ָ̬ʾ
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_system_NetLedToggle(void)
{
	if(sysCfg.parameter.data_socket == SOCK_TCPS)
	{
		bsp_LedOff(3);
		bsp_LedOff(4);
		bsp_LedToggle(2);				
	}
	else
	{
    bsp_LedOff(2);	
		bsp_LedOff(4);
		bsp_LedToggle(3);	
	}
}

/*
*********************************************************************************************************
 * ��������app_system_LedOn
 * ����  ������ָʾ
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_system_LedOn(void)
{	
	if(sysCfg.parameter.data_socket == SOCK_TCPS)
	{
		bsp_LedOff(3);	
		bsp_LedOn(2);				
	}
	else
	{
		bsp_LedOff(2);	
		bsp_LedOn(3);	
	}	
}

/*
*********************************************************************************************************
 * ��������app_system_NetPublic
 * ����  �����Ӳ�������
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void app_system_NetPublic(void)
{
	uint8_t mac[6];
	uint8_t cip[4];
	uint8_t sip[4];
	uint8_t nrf[6];
  uint8_t startstatus = 0;
	
	char dev[30] = {0};
	char ble[30] = {0};
	char link[30] = {0};
	char netparm[500] = {0};
	//char *state[6] = {"UNRST","PORRST","SFTRST","IWDGRS","WWDGRS","LPWRRSTF"};
	char *state[6] = {"unrst","porrst","sftrst","iwdrst","wwdrst","lpwrst"};
	
	uint32_t CSR = RCC->CSR;
	uint32_t reset_state[5] = {PORRSTF_MASK,SFTRST_MASK,IWDGRST_MASK,WWDGRST_MASK,LPWRRSTF_MASK};
		
  /*startstatus: 1����Դ��λ 2�������λ 3���������Ź���λ 4�����ڿ��Ź���λ 5���͹��ĸ�λ*/	
	for(uint8_t i = 0; i < 5; i++)
	{
		if(CSR & reset_state[i])
		{
			startstatus = i + 1;
			break;
		}
	}
  
	#if APP_DEBUG
	printf("startstatus:%s\r\n",state[startstatus]);
	#endif
	
	RCC_ClearFlag();
	
	memcpy(sip,sysCfg.parameter.server_ip,4);  
	memcpy(cip,sysCfg.parameter.client_ip,4);  
	memcpy(mac,sysCfg.parameter.client_mac,6); 
	memcpy(nrf,&sysCfg.parameter.nrfstation[2],6);
	
	sprintf(dev,"0000%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	sprintf(ble,"0000%02x%02x%02x%02x%02x%02x",nrf[0],nrf[1],nrf[2],nrf[3],nrf[4],nrf[5]);
	sprintf(link,"%d.%d.%d.%d:%d",sip[0],sip[1],sip[2],sip[3],sysCfg.parameter.server_port);
	
	sprintf(netparm,"{\"gid\":\"%s\",\"bid\":\"%s\",\"cip\":\"%d.%d.%d.%d\",\"mac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"connect\":\"%s\",\"sver\":\"%s\",\"hver\":\"%s\",\"sta\":\"%s\"}",
	dev,ble,cip[0],cip[1],cip[2],cip[3],mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],link,SOFTWARE_VERSION,HARDWARE_VERSION,state[startstatus]);
		
	//printf("%s\r\n",netparm);	
	
	mqtt_publish(SOCK_TCPS,SOFTVERSION_TOPIC,netparm,strlen(netparm));//�ϱ��汾��-
}

/*
*********************************************************************************************************
 * ��������app_system_TcpsStart
 * ����  �������������ķ���
 * ����  ����
 * ����  : ��
*********************************************************************************************************
 */
void static app_system_TcpsStart(void)
{
	/*������������*/
	if(0 != mqtt_subscrib(SOCK_TCPS,bleupdata_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",bleupdata_topic);
 		#endif  
	}	
	
	/*������������*/
	if(0 != mqtt_subscrib(SOCK_TCPS,gateupdata_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",gateupdata_topic);
 		#endif  
	}	
	
	/*�����ֻ���������*/
	if(0 != mqtt_subscrib(SOCK_TCPS,bracelet_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",bracelet_topic);
 		#endif  
	}	
			
	/*�����豸��Ϣ����*/
	if(0 != mqtt_subscrib(SOCK_TCPS,devcfgpara_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",devcfgpara_topic);
 		#endif  
	}	
			
	/*�����ű��������*/
//	if(0 != mqtt_subscrib(SOCK_TCPS,beaconmanagement_topic))   
//	{
//		#if APP_DEBUG
//		printf("subscrib topic:%s\r\n",beaconmanagement_topic);
//		#endif
//	}

	/*������������*/
	if(0 != mqtt_subscrib(SOCK_TCPS,gatereset_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",devcfgpara_topic);
 		#endif  
	}	
	
	#ifdef MCBR03
	app_plat_SHTDataPublish();  /*�ϱ���ʪ������*/
	#endif
}

/*
*********************************************************************************************************
 * ��������app_system_Subscrib
 * ����  �����ĵ���������
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void static app_system_Subscrib(uint8_t socket)
{
	/*��Ϣ����*/
	if(0 != mqtt_subscrib(socket,sms_topic)) 
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",sms_topic);
		#endif
	}	
	
	/*�û���Ϣ����*/
	if(0 != mqtt_subscrib(socket,userinfo_topic))      
	{
		#if APP_DEBUG
		printf("subscrib topic:%s\r\n",userinfo_topic);
 		#endif  
	}	
}

/*
*********************************************************************************************************
 * ��������app_system_Subscrib
 * ����  �����ĵ���������
 * ����  ��socket,�����˿�
 * ����  : ��
*********************************************************************************************************
*/
void app_system_MqttConnect(uint8_t socket)
{
	int rc = 0;

	if(sysCfg.parameter.dhcp == NETINFO_STATIC)
	{
		rc = DHCP_IP_LEASED;
		set_netparm();
	}
	
	while(rc!=DHCP_IP_LEASED)
	{
		#if APP_DEBUG
		printf("DHCP_run.\r\n");
		#endif
		IWDG_Feed();  /*ι��*/
		vTaskDelay(2000); 
		rc = DHCP_run();			
	}

	transport_close(socket);   //�رն˿�
	
	#if APP_DEBUG
	printf("close socket:%d\r\n",socket);
	#endif 

	if(socket == SOCK_TCPS)      /*������������*/
	{
		if(0 != mqtt_connect(SOCK_TCPS,default_server_ip, SERVER_PORT, (char *)default_server_user , (char *)default_server_pass )) //���ӷ�����	
		{
			vTaskDelay(3000);
			app_system_TcpsStart();		
			if(sysCfg.parameter.data_socket == SOCK_TCPS)
			{
				app_system_Subscrib(SOCK_TCPS);
			}
		}
	}
	else if(socket == SOCK_TCP) /*���ӵ�����������*/
	{
		if(0 != mqtt_connect(SOCK_TCP, sysCfg.parameter.server_ip, sysCfg.parameter.server_port, (char *)sysCfg.parameter.server_user , (char *)sysCfg.parameter.server_pass ))
		{
			vTaskDelay(1000);	
			app_system_Subscrib(SOCK_TCP);
		}
	}
}

/*
*********************************************************************************************************
 * ��������app_system_UpdataBle
 * ����  ��������������
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_system_UpdataBle(void)
{
	uint16_t len = 0;
	uint8_t  byte = 0;
	uint8_t  recbleacklen = 0;
	
  uint8_t  buf[1200] = {0};               /*�����������ݻ���*/
	uint8_t  recbleackbuf[100] = {0};       /*�����������ݻ�ִ*/
	
	if((len = getSn_RX_RSR(SOCK_BLE)) > 0)
	{
		recv(SOCK_BLE,buf,len);
						
		while(comGetChar(NRF_PORT,&byte));  
		comSendBuf(NRF_PORT,buf,len);
		
		vTaskDelay(1000);
		
		while(comGetChar(NRF_PORT,&recbleackbuf[recbleacklen++]));
		
		#if 0
		printf("len:%d  ",len);
	  for(uint8_t i = 0; i < recbleacklen - 1; i++)
		{
				printf("%02x ",recbleackbuf[i]);
		}
		printf("\r\n");
		#endif
		
		if(recbleacklen > 1)
		{
			send(SOCK_BLE,recbleackbuf,recbleacklen-1);
		}
	}
}
/*
*********************************************************************************************************
 * ��������app_system_CheckID
 * ����  �����ڲ�ѯ�豸ID
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_system_CheckID(void)
{
	uint8_t i = 0;
	char recebuf[100];
	
	/*���˿�����*/
	while(comGetChar(COM1,(uint8_t *)&recebuf[i++]))
  {
		i %= 100;
	}
	recebuf[i] = '\0';
	
	if(NULL != strstr(recebuf,"ID=?"))
	{
		if(sysCfg.parameter.register_flag == REGISTER)
		{
			printf("ID=0000%02X%02X%02X%02X%02X%02X:0000%02X%02X%02X%02X%02X%02X:\r\n",sysCfg.parameter.client_mac[0],
			sysCfg.parameter.client_mac[1],sysCfg.parameter.client_mac[2],sysCfg.parameter.client_mac[3],sysCfg.parameter.client_mac[4],
			sysCfg.parameter.client_mac[5],sysCfg.parameter.nrfstation[2],sysCfg.parameter.nrfstation[3],sysCfg.parameter.nrfstation[4],
			sysCfg.parameter.nrfstation[5],sysCfg.parameter.nrfstation[6],sysCfg.parameter.nrfstation[7]);
		}
	}
}

/*
*********************************************************************************************************
 * ��������app_system_Start
 * ����  ��ϵͳ�������Ӷ���
 * ����  ����
 * ����  : ��
*********************************************************************************************************
*/
void app_system_Start(void)
{
	/*������������*/
	if(0 != mqtt_connect(SOCK_TCPS,default_server_ip, SERVER_PORT, (char *)default_server_user , (char *)default_server_pass)) //���ӷ�����	
	{
		vTaskDelay(1000);
		app_system_TcpsStart();
		
		if(sysCfg.parameter.data_socket == SOCK_TCPS)
		{
			app_system_Subscrib(SOCK_TCPS);
			return;
		}
	}
	
	/*���ӵ���������������������*/
	vTaskDelay(1000);
	if(0 != mqtt_connect(SOCK_TCP, sysCfg.parameter.server_ip, sysCfg.parameter.server_port, (char *)sysCfg.parameter.server_user , (char *)sysCfg.parameter.server_pass))
	{
		vTaskDelay(1000);	
		app_system_Subscrib(SOCK_TCP);
	}
}

