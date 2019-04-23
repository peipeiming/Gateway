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
typedef struct _DateTime	/*此结构体定义了NTP时间同步的相关变量*/
{
  volatile uint8 year[2];																			  /*年为2字节 分为高低两位 */
  volatile uint8 month;																				  /*	月 */
  volatile uint8 day;																					  /*	天 */
  volatile uint8 hour;																					/*	时 */
  volatile uint8 minute;																				/*	分 */
  volatile uint8 second;																				/*	秒 */
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
