/**
  ******************************************************************************
  * @file    timer.c
  * $Author: �ɺ�̤ѩ $
  * $Revision: 17 $
  * $Date:: 2014-10-25 11:16:48 +0800 #$
  * @brief   ������غ���.
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
#ifndef _TIMER_H
#define _TIMER_H
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stm32f10x.h>

/* Exported Functions --------------------------------------------------------*/
void Timer_Config(void);
void Timer_Start(void);
void Timer_Stop(void);

void key_timer_Stop(void);
void key_timer_Start(void);

#endif /*_TIMER_H*/

/*********************************END OF FILE**********************************/
