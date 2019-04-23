/**
  ******************************************************************************
  * @file    transport.h
  * $Author: 飞鸿踏雪 $
  * $Revision: 17 $
  * $Date:: 2012-07-06 11:16:48 +0800 #$
  * @brief   以太网收发相关函数包装.
  ******************************************************************************
  * @attention
  *
  *<h3><center>&copy; Copyright 2009-2012, EmbedNet</center>
  *<center><a href="http:\\www.embed-net.com">http://www.embed-net.com</a></center>
  *<center>All Rights Reserved</center></h3>
  * 
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TRANSPORT_H
#define __TRANSPORT_H
/* Includes ------------------------------------------------------------------*/

/* Exported Functions --------------------------------------------------------*/

int transport_sendPacketBuffer(unsigned char socket,unsigned char* buf, int buflen);
int transport_getdata0(unsigned char* buf, int count);
int transport_getdata1(unsigned char* buf, int count);
int transport_open(unsigned char sock, unsigned char * addr, unsigned int port);
int transport_close(unsigned char socket);
void network_init(void);
void net_init(void);
void get_netparm(void);
void set_netparm(void);

#endif /* __MAIN_H */

/*********************************END OF FILE**********************************/
