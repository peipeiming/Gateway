/*MQTT��W5500�����Ľӿں�����ֻ��Ҫ��װ�ĸ�����*/
#include "includes.h"
extern CONFIG_MSG  ConfigMsg;
uint8_t gDATABUF[DATA_BUF_SIZE];
wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc,0x06, 0xab, 0xcd},
                            .ip = {192, 168, 0, 198},	//����IP
                            .sn = {255,255,255,0},		//����
                            .gw = {192, 168, 0, 1},		//����
                            .dns = {114,114,114,114},	//�޸ĳ��Լ������DNS
                            .dhcp = NETINFO_STATIC };


uint8_t domain_name[]="www.baidu.com";//֧����������
void set_netparm(void)//��WEB �����õ�IPд��W5500
{
	memcpy(gWIZNETINFO.gw,sysCfg.parameter.gw,4);
	memcpy(gWIZNETINFO.ip,sysCfg.parameter.ip,4);
	memcpy(gWIZNETINFO.sn,sysCfg.parameter.sub,4);
	network_init();
}
void get_netparm(void)//����ǰ�������д�� �ύ��WEB�Ľṹ����
{
	memcpy(ConfigMsg.mac,gWIZNETINFO.mac,6);
	memcpy(ConfigMsg.gw,gWIZNETINFO.gw,4);
	memcpy(ConfigMsg.lip,gWIZNETINFO.ip,4);
	memcpy(ConfigMsg.sub,gWIZNETINFO.sn,4);
	//ConfigMsg.port=sysCfg.parameter.server_port;
}
/*DHCP_Run()�����л���ô˺���*/
void my_ip_conflict(void)
{
	printf("CONFLICT IP from DHCP\r\n");
	//halt or reset or any...
	while(1); // this example is halt.
}

/*******************************************************
 * @ brief Call back for ip assing & ip update from DHC7��P
 *******************************************************/
/*DHCP_Run()�����л���ô˺���*/
void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);
   getGWfromDHCP(gWIZNETINFO.gw);
   getSNfromDHCP(gWIZNETINFO.sn);
   getDNSfromDHCP(gWIZNETINFO.dns);
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   /* Network initialization */
   network_init();      // apply from dhcp
	 #if APP_DEBUG
   printf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());
	 #endif
}

void net_init(void)
{
	uint8_t tmp;//
	uint8_t memsize[2][8] = { {4,4,1,2,2,1,1,1},{4,4,1,2,2,1,1}};
		
	memcpy(gWIZNETINFO.mac,sysCfg.parameter.client_mac,6);
		
		// First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP 
	/* Critical section callback */
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);	//ע���ٽ�������
	/* Chip selection call back */
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);//ע��SPIƬѡ�źź���
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // CS must be tried with LOW.
#else
   #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
      #error "Unknown _WIZCHIP_IO_MODE_"
   #else
      reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
   #endif
#endif
	/* SPI Read & Write callback function */
	reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);	//ע���д����

	/* WIZCHIP SOCKET Buffer initialize */
	if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
		 #if APP_DEBUG
		 printf("WIZCHIP Initialized fail.\r\n");
		 #endif
		 while(1);
	}

	/* PHY link status check */
	do{
		 if(ctlwizchip(CW_GET_PHYLINK, (void*)&tmp) == -1){
			  #if APP_DEBUG
				printf("Unknown PHY Link stauts.\r\n");
		    #endif
		 }
		 IWDG_Feed();  /*ι��*/
	}while(tmp == PHY_LINK_OFF);
	setSHAR(gWIZNETINFO.mac);//����DHCP֮ǰ����������MAC��ַ
	DHCP_init(SOCK_DHCP, gDATABUF);
	/*ע��ip_assign,ip_updata,ip_conflict������DHCP_Run�󣬻���ݵ�ǰ״̬�ص���Ӧ�Ĵ�����*/
	reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
  //����Ĭ��IP Ϊ192.168.0.198,��DHCP ���䲻��IPʱ�������������β��� ����Ϊ��̬IP
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
	//��������ʱ�䣬Ĭ��Ϊ2000(200ms) 
	//ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
	setRTR(0x07d0);
	//�������Դ�����Ĭ��Ϊ8�� 
	//����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
	setRCR(5);
	//setIMR(IM_IR7);//����IP��ͻ�쳣�ж�
	
   setSIMR(0x07);    //����Socket 0��1��2 �ж�
   setSn_IMR(SOCK_TCPS,Sn_IR_RECV);  //���������������ж�
	 setSn_IMR(SOCK_TCP,Sn_IR_RECV);   //�������������������ж�
	 setSn_IMR(SOCK_BLE,Sn_IR_RECV);   //���������������������ж�
}

void network_init(void)
{
  uint8_t tmpstr[6] = {0};
	wiz_NetInfo netinfo;

	// Set Network information from netinfo structure
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

	// Get Network information
	ctlnetwork(CN_GET_NETINFO, (void*)&netinfo);

	// Display Network Information
	ctlwizchip(CW_GET_ID,(void*)tmpstr);

	if(netinfo.dhcp == NETINFO_DHCP) 
	{
		#if APP_DEBUG
		printf("\r\n=== %s NET CONF : DHCP ===\r\n",(char*)tmpstr);
		#endif
	}
	else 
	{
		#if APP_DEBUG
		printf("\r\n=== %s NET CONF : Static ===\r\n",(char*)tmpstr);
	  #endif
	}

	memcpy(sysCfg.parameter.client_ip,netinfo.ip,4);
	
	#if APP_DEBUG
	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",netinfo.mac[0],netinfo.mac[1],netinfo.mac[2],
			netinfo.mac[3],netinfo.mac[4],netinfo.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", netinfo.ip[0],netinfo.ip[1],netinfo.ip[2],netinfo.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", netinfo.gw[0],netinfo.gw[1],netinfo.gw[2],netinfo.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", netinfo.sn[0],netinfo.sn[1],netinfo.sn[2],netinfo.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", netinfo.dns[0],netinfo.dns[1],netinfo.dns[2],netinfo.dns[3]);
	printf("===========================\r\n");
	#endif
}

/**
  * @brief  ͨ��TCP��ʽ�������ݵ�TCP������
  * @param  buf �����׵�ַ
  * @param  buflen ���ݳ���
  * @retval С��0��ʾ����ʧ��
  */
int transport_sendPacketBuffer(unsigned char socket,unsigned char* buf, int buflen)
{
  return send(socket,buf,buflen);
}
/**
  * @brief  ������ʽ����TCP���������͵�����
  * @param  buf ���ݴ洢�׵�ַ
  * @param  count ���ݻ���������
  * @retval С��0��ʾ��������ʧ��
  */
int transport_getdata0(unsigned char* buf, int count)
{
  return recv(SOCK_TCPS,buf,count);
}

int transport_getdata1(unsigned char* buf, int count)
{
  return recv(SOCK_TCP,buf,count);
}

/**
  * @brief  ��һ��socket�����ӵ�������
  * @param  ��
  * @retval С��0��ʾ��ʧ��
  */
int transport_open(unsigned char sock, unsigned char * addr, unsigned int port)
{
	static uint16_t numport=1000;
  int32_t ret;
  //�½�һ��Socket���󶨱��ض˿�
	printf("socket:%d  port:%d\r\n",sock,numport);
  ret = socket(sock,Sn_MR_TCP,numport++,0x00);
  if(ret != sock){
		#if APP_DEBUG
    printf("%d:Socket Error\r\n",sock);
		#endif
   return 0; 
  }
	if(numport>=32000)
		numport=4096;
  //����TCP������
	vTaskDelay(2000);
	IWDG_Feed();  /*ι��*/
  ret = connect(sock,addr,port);
  if(ret != SOCK_OK){
		#if APP_DEBUG
    printf("%d:Socket Connect Error\r\n",sock);
	  #endif
		//printf("%d",ret);
		return 0;  
  }	
	return 1;
}
/**
  * @brief  �ر�socket
  * @param  ��
  * @retval С��0��ʾ�ر�ʧ��
  */
int transport_close(unsigned char socket)
{
  close(socket);
  return 0;
}
