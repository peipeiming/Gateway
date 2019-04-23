#ifndef	__NTP_H__
#define	__NTP_H__
#include "types.h"
#include "stm32f10x.h"
/* for ntpclient */ 

#define SECS_PERDAY     	    86400UL             	  /* seconds in a day = 60*60*24 */
#define UTC_ADJ_HRS         	9              	        /* SEOUL : GMT+9 */
#define EPOCH                 1900                    /* NTP start year */


extern uint8 ntp_server_ip[4];
extern uint8 ntp_port;
typedef signed char s_char;
typedef unsigned long long tstamp;
typedef unsigned int tdist;

#pragma pack(1)
typedef struct _NPTformat
{
 	uint8  dstaddr[4];     															/* destination (local) address */
	char    version;       														 	/* version number */
	char    leap;          												 			/* leap indicator */
	char    mode;           														/* mode */
	char    stratum;       												 			/* stratum */
	char    poll;           														/* poll interval */
	s_char  precision;     								 							/* precision */
	tdist   rootdelay;      														/* root delay */
	tdist   rootdisp;       														/* root dispersion */
	char    refid;          														/* reference ID */
	tstamp  reftime;        														/* reference time */
	tstamp  org;            														/* origin timestamp */
	tstamp  rec;            														/* receive timestamp */
	tstamp  xmt;            														/* transmit timestamp */
} NPTformat;
#pragma pack()

#pragma pack(1)
typedef struct _DateTime	/*�˽ṹ�嶨����NTPʱ��ͬ������ر���*/
{
  volatile uint8 year[2];																			  /*��Ϊ2�ֽ� ��Ϊ�ߵ���λ */
  volatile uint8 month;																				  /*	�� */
  volatile uint8 day;																					  /*	�� */
  volatile uint8 hour;																					/*	ʱ */
  volatile uint8 minute;																				/*	�� */
  volatile uint8 second;																				/*	�� */
} timepara;
typedef union 
{
	timepara time;
	uint8_t data[7];
}DateTime;
#pragma pack()

void do_ntp_client(void);
void get_ntp_time(DateTime *time);

#endif
