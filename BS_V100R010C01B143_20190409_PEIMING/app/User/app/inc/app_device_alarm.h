#ifndef APP_ERROR_H
#define APP_ERROR_H

#include "stm32f10x.h"

#ifndef ERRORLOG
#define ERRORLOG   1
#endif

//#ifndef APP_DEBUG
//#define APP_DEBUG  1
//#endif

/*�澯�ȼ�*/
#define INFO   0x01

#define WARN   0x02
#define ERR    0x03

/*������*/
#define ELECT_WARN 502
#define MESEG_WARN 603
#define SOS_WARN 	 606
#define SOS_QUIT 	 607
/*�豸����*/
#define STATION    0x01  /*��վ*/
#define BRACELET   0x02  /*�ֻ�*/
#define BEACON     0x03  /*�ű�*/

#define BEACON_WARN_VOLTAGE    2.3   /*�ű�澯��ѹ 2.3v*/
#define BRACELET_WARN_ELECT    5     /*�ֻ��澯���� 10%*/

void ErrorLog(uint8_t devtype,uint8_t *dev,uint8_t err_level,uint16_t err_code,uint8_t err_para_count,uint8_t *para);

#endif

