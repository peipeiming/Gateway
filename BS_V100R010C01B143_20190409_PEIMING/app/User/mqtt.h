#ifndef __MQTT_H
#define __MQTT_H

#include "stm32f10x.h"

void mqtt_disconnect(void);
int Heartbeat(uint8_t socket);
int mqtt_subscrib(uint8_t socket,char *pTopic);
int mqtt_publish(uint8_t socket,char *pTopic,char *pMessage,int msglen);
int mqtt_connect(uint8_t socket,uint8_t * addr, uint16_t port, char *name, char *word);

#endif 

