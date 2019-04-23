/*MQTTӦ�ò㺯��,*/
#include "includes.h"

void mqtt_disconnect(void)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(SOCK_TCPS, buf, len);		//���ط��͵ĳ���
	if(sysCfg.parameter.data_socket == SOCK_TCP)
	{
		transport_sendPacketBuffer(SOCK_TCP, buf, len);
	}
}	

/**
  * @brief  ����
  * @param  ��
  * @param  ��
  * @retval 0 ��ʧ��  1:�ɹ�
  */
int Heartbeat(uint8_t socket)
{
	uint8_t len,rc;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf,buflen);
	rc = transport_sendPacketBuffer(socket,buf, len);//���ط��͵ĳ���
	if(rc != len)
	{
		bsp_LedOff(2);     /*��ɫ*/
		bsp_LedOff(3);     /*��ɫ*/
		bsp_LedOff(4);     /*��ɫ*/
		#if APP_DEBUG
		printf("%d:Heartbeat fail\n\r",socket);
		#endif
		return 0;	
	}
	return 1;
}	
/**
  * @brief  ����MQTT������
  * @param  name:�û��� 
  * @param  word������
  * @retval ��
  */
int mqtt_connect(uint8_t socket,uint8_t * addr, uint16_t port, char *name, char *word)
{
	int32_t len,rc;
	unsigned char buf[512];
	int buflen = sizeof(buf);
	
	while(!transport_open(socket,addr, port));  //�򿪱��ض˿ڣ����ӵ�Զ�̶˿�
	  	
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;

	uint16_t crcdata;
	uint8_t will_topic_str[17]={0x0D,0x00,0x59,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01};

	data.clientID.cstring = (char *)sysCfg.parameter.client_id;
  data.keepAliveInterval = 120;
  data.cleansession = 1;
	data.username.cstring = name;
	data.password.cstring = word;
	
	/************** modify 2018/07/31 by PM will config ************************/
	memcpy(&will_topic_str[5],sysCfg.parameter.client_mac,6);
	crcdata=app_plat_usMBCRC16(will_topic_str,15);
	memcpy(&will_topic_str[15],(uint8_t *)&crcdata,2);
	
	data.willFlag = 1;
	data.will.topicName.cstring = WILL_TOPIC;
	data.will.message.lenstring.len = sizeof(will_topic_str);
	data.will.message.lenstring.data = (char *)will_topic_str;
	/***************************************************************************/

	len = MQTTSerialize_connect(buf, buflen, &data); 
	rc = transport_sendPacketBuffer(socket,buf,len);	//���ط��͵ĳ���
  if(rc != len){
		#if APP_DEBUG
    printf("Connect:connect transport_sendPacketBuffer error code:%d\n\r",rc);
		#endif
		if(socket == SOCK_TCPS)
		{
			while(1);
		}
		return 0;	
  }

	return rc;
}


/**
  * @brief  �����������������һ����Ϣ
  * @param  pTopic ��Ϣ����
  * @param  pMessage ��Ϣ����
  * @retval  0��ʾ����ʧ�� 1:���ͳɹ�
  */
int mqtt_publish(uint8_t socket,char *pTopic,char *pMessage,int msglen)
{
  int32_t len,rc;
  unsigned char buf[256];
  MQTTString topicString = MQTTString_initializer;
	
	#if 0  /*modify by pei 2018-1-29*/
  int msglen = strlen(pMessage);
	#endif
	
  int buflen = sizeof(buf);
	/*publishh*/
  topicString.cstring = pTopic;
  len= MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (unsigned char*)pMessage, msglen); /* 2 */
  //len += MQTTSerialize_disconnect(buf + len, buflen - len); /* 3 */
  rc = transport_sendPacketBuffer(socket,buf,len);
	if (rc == len)
	{
		#if 0
		printf("Published OK\r\n");
		#endif
		return 1;
	}
	else
	{
		//printf("Publish failed\r\n");
		return 0;
	}
	
//�����Ҫ��������Ϣ�����رն˿ڣ���Ҫһ����ʱ QOS=1��������Ϣʱ���������˻᷵��PUBACK
//	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
//	{
//	
//	}
	
}
/**
  * @brief  �����������һ����Ϣ���ú�������ΪTCP�������ݺ���������
  * @param  pTopic ��Ϣ���⣬����
  * @retval 0��ʾ������Ϣʧ�ܣ�rc ���͵����ݳ���
  */
int mqtt_subscrib(uint8_t socket,char *pTopic)
{
	MQTTString topicString = MQTTString_initializer;
	unsigned char buf[128];
	int buflen = sizeof(buf);
	int rc = 0;
	int msgid = 1;
	int req_qos =1;
	int len = 0;
	/* subscribe */
	topicString.cstring = pTopic;
	len = MQTTSerialize_subscribe(buf, buflen, 0, msgid, 1, &topicString, &req_qos);
	rc = transport_sendPacketBuffer(socket,buf, len);
  if(rc != len)
	{
		#if APP_DEBUG
    printf("Subscrib:connect transport_sendPacketBuffer error\n\r");
		printf("rc:%d\r\n",rc);
		#endif
		if(socket == SOCK_TCPS)
		{
			while(1);
		}
		
		bsp_LedOn(4);
		vTaskDelay(1000);
		bsp_LedOff(4);
		return 0;
  }
	
  return rc;
}


