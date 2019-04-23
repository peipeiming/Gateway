/*MQTT应用层函数,*/
#include "includes.h"

void mqtt_disconnect(void)
{
	uint8_t len;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	
	len = MQTTSerialize_disconnect(buf,buflen);
	transport_sendPacketBuffer(SOCK_TCPS, buf, len);		//返回发送的长度
	if(sysCfg.parameter.data_socket == SOCK_TCP)
	{
		transport_sendPacketBuffer(SOCK_TCP, buf, len);
	}
}	

/**
  * @brief  心跳
  * @param  无
  * @param  无
  * @retval 0 ：失败  1:成功
  */
int Heartbeat(uint8_t socket)
{
	uint8_t len,rc;
	uint8_t buf[100]={0};
	uint8_t buflen = sizeof(buf);
	len = MQTTSerialize_pingreq(buf,buflen);
	rc = transport_sendPacketBuffer(socket,buf, len);//返回发送的长度
	if(rc != len)
	{
		bsp_LedOff(2);     /*蓝色*/
		bsp_LedOff(3);     /*绿色*/
		bsp_LedOff(4);     /*红色*/
		#if APP_DEBUG
		printf("%d:Heartbeat fail\n\r",socket);
		#endif
		return 0;	
	}
	return 1;
}	
/**
  * @brief  连接MQTT服务器
  * @param  name:用户名 
  * @param  word：密码
  * @retval 无
  */
int mqtt_connect(uint8_t socket,uint8_t * addr, uint16_t port, char *name, char *word)
{
	int32_t len,rc;
	unsigned char buf[512];
	int buflen = sizeof(buf);
	
	while(!transport_open(socket,addr, port));  //打开本地端口，连接到远程端口
	  	
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
	rc = transport_sendPacketBuffer(socket,buf,len);	//返回发送的长度
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
  * @brief  向代理（服务器）发送一个消息
  * @param  pTopic 消息主题
  * @param  pMessage 消息内容
  * @retval  0表示发送失败 1:发送成功
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
	
//如果需要发送完消息，即关闭端口，需要一定延时 QOS=1，发布消息时，服务器端会返回PUBACK
//	 if (MQTTPacket_read(buf, buflen, transport_getdata) == PUBACK) 	
//	{
//	
//	}
	
}
/**
  * @brief  向服务器订阅一个消息，该函数会因为TCP接收数据函数而阻塞
  * @param  pTopic 消息主题，传入
  * @retval 0表示订阅消息失败，rc 发送的数据长度
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


