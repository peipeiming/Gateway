#include "ntp.h"
#include "bsp.h"
#include "types.h"
#include "socket.h"
#include "w5500.h"
#include "includes.h"

#include <string.h>
#include <stdio.h>

static  DateTime nowdate;
static  uint8 time_zone = 39;
static  uint8 ntp_port = 123; 																				/*NTP服务器端口号*/
static  uint8 ntp_message[48]={0};
static  uint8 ntp_server_ip[4]={120,25,108,11}; 										  /*阿里云NTP服务器*/

/**
*@brief			计算日期时间
*@param			secondsUTC 世界标准时间 
*@return		无
*/
static void calc_date_time(tstamp seconds)
{
	uint8 yf=0;
	uint32 p_year_total_sec;
	uint32 r_year_total_sec;
	tstamp n=0,d=0,total_d=0,rz=0;
	uint16 y=0,r=0,yr=0;
	signed long long yd=0;
	
	n = seconds;
	total_d = seconds/(SECS_PERDAY);
	d=0;
	p_year_total_sec=SECS_PERDAY*365;
	r_year_total_sec=SECS_PERDAY*366;
	while(n>=p_year_total_sec) 
	{
		if((EPOCH+r)%400==0 || ((EPOCH+r)%100!=0 && (EPOCH+r)%4==0))
		{
			n = n -(r_year_total_sec);
			d = d + 366;
		}
		else
		{
			n = n - (p_year_total_sec);
			d = d + 365;
		}
		r+=1;
		y+=1;

	}

	y += EPOCH;

	nowdate.time.year[1] = (uint8)((y & 0xff00)>>8);
	nowdate.time.year[0] = (uint8)(y & 0xff);

	yd=0;
	yd = total_d - d;

	yf=1;
	while(yd>=28) 
	{
	
		if(yf==1 || yf==3 || yf==5 || yf==7 || yf==8 || yf==10 || yf==12)
		{
			yd -= 31;
			if(yd<0)break;
			rz += 31;
		}
		
		if (yf==2)
		{
			if (y%400==0 || (y%100!=0 && y%4==0)) 
			{
				yd -= 29;
				if(yd<0)break;
				rz += 29;
			}
			else 
			{
				yd -= 28;
				if(yd<0)break;
				rz += 28;
			}
		} 
		if(yf==4 || yf==6 || yf==9 || yf==11 )
		{
			yd -= 30;
			if(yd<0)break;
			rz += 30;
		}
		yf += 1;

	}
	nowdate.time.month=yf;
	yr = total_d-d-rz;
	
	yr += 1;
	
	nowdate.time.day=yr;
	
	//calculation for time
	seconds = seconds%SECS_PERDAY;
	nowdate.time.hour = seconds/3600;
	nowdate.time.minute = (seconds%3600)/60;
	nowdate.time.second = (seconds%3600)%60;
}

/**
*@brief			改编日期时间为秒
*@param		  seconds： 
*@return		无
*/
static tstamp change_datetime_to_seconds(void) 
{
	tstamp seconds=0;
	uint32 total_day=0;
	uint16 i=0,run_year_cnt=0,l=0;
	
	l = nowdate.time.year[0];//high
	
	l = (l<<8);
	
	l = l + nowdate.time.year[1];//low
	
	
	for(i=EPOCH;i<l;i++)
	{
		if((i%400==0) || ((i%100!=0) && (i%4==0))) 
		{
			run_year_cnt += 1;
		}
	}
	
	total_day=(l-EPOCH-run_year_cnt)*365+run_year_cnt*366;
	
	for(i=1;i<=nowdate.time.month;i++)
	{
		if(i==5 || i==7 || i==10 || i==12)
		{
			total_day += 30;
		}
		if (i==3)
		{
			if (l%400==0 && l%100!=0 && l%4==0) 
			{
				total_day += 29;
			}
			else 
			{
				total_day += 28;
			}
		} 
		if(i==2 || i==4 || i==6 || i==8 || i==9 || i==11)
		{
			total_day += 31;
		}
	}
	
	seconds = (total_day+nowdate.time.day-1)*24*3600;  
	seconds += nowdate.time.second;
	seconds += nowdate.time.minute*60;
	seconds += nowdate.time.hour*3600;
	
	return seconds;
}

/**
*@brief		从NTP服务器获取时间
*@param		buf：存放缓存
*@param		idx：定义存放数据起始位置
*@return	无
*/
static void get_seconds_from_ntp_server(uint8* buf,uint16 idx)
{
	tstamp seconds = 0;
	uint8 i=0;
	for (i = 0; i < 4; i++)
	{
		seconds = (seconds << 8) | buf[idx + i];
	}
	switch (time_zone)
	{
		case 0:
			seconds -=  12*3600;
		break;
		case 1:
			seconds -=  11*3600;
		break;
		case 2:
			seconds -=  10*3600;
		break;
		case 3:
			seconds -=  (9*3600+30*60);
		break;
		case 4:
			seconds -=  9*3600;
		break;
		case 5:
		case 6:
			seconds -=  8*3600;
		break;
		case 7:
		case 8:
			seconds -=  7*3600;
		break;
		case 9:
		case 10:
			seconds -=  6*3600;
		break;
		case 11:
		case 12:
		case 13:
			seconds -= 5*3600;
		break;
		case 14:
			seconds -=  (4*3600+30*60);
		break;
		case 15:
		case 16:
			seconds -=  4*3600;
		break;
		case 17:
			seconds -=  (3*3600+30*60);
		break;
		case 18:
			seconds -=  3*3600;
		break;
		case 19:
			seconds -=  2*3600;
		break;
		case 20:
			seconds -=  1*3600;
		break;
		case 21:                           
		case 22:
		break;
		case 23:
		case 24:
		case 25:
			seconds +=  1*3600;
		break;
		case 26:
		case 27:
			seconds +=  2*3600;
		break;
		case 28:
		case 29:
			seconds +=  3*3600;
		break;
		case 30:
			seconds +=  (3*3600+30*60);
		break;
		case 31:
			seconds +=  4*3600;
		break;
		case 32:
			seconds +=  (4*3600+30*60);
		break;
		case 33:
			seconds +=  5*3600;
		break;
		case 34:
			seconds +=  (5*3600+30*60);
		break;
		case 35:
			seconds +=  (5*3600+45*60);
		break;
		case 36:
			seconds +=  6*3600;
		break;
		case 37:
			seconds +=  (6*3600+30*60);
		break;
		case 38:
			seconds +=  7*3600;
		break;
		case 39:
			seconds +=  8*3600;
		break;
		case 40:
			seconds +=  9*3600;
		break;
		case 41:
			seconds +=  (9*3600+30*60);
		break;
		case 42:
			seconds +=  10*3600;
		break;
		case 43:
			seconds +=  (10*3600+30*60);
		break;
		case 44:
			seconds +=  11*3600;
		break;
		case 45:
			seconds +=  (11*3600+30*60);
		break;
		case 46:
			seconds +=  12*3600;
		break;
		case 47:
			seconds +=  (12*3600+45*60);
		break;
		case 48:
			seconds +=  13*3600;
		break;
		case 49:
			seconds +=  14*3600;
		break;
	}
	
	calc_date_time(seconds); 																									/*由UTC时间计算日期*/
}




/*获取ntp网络时间*/
void get_ntp_time(DateTime *time)
{
	memcpy(time->data,nowdate.data,sizeof(nowdate.data));
}
	
/**
*@brief			执行NTP Client主函数
*@param			无
*@return		无
*/
void do_ntp_client(void)
{
	uint8_t i=0;
	uint8_t data_buf[48];
	uint8_t check_results;
	uint8_t timepack[12]={0x5A,0x47,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x0,0x00};	
	
	uint16 len;
	uint16 destport;
	uint16 startindex = 40;
		
 	uint32 destip = 0;		/*回复包中时间数据首地址*/
//  char buf[100];

	static uint8_t lastmin = 0;
	
	switch(getSn_SR(SOCK_NTP))
	{
		/*发送请求到NPT服务器*/
		case SOCK_UDP:	
      ntp_message[0]=0xA3;
			sendto(SOCK_NTP,ntp_message,sizeof(ntp_message),ntp_server_ip, ntp_port);
			if ((len = getSn_RX_RSR(SOCK_NTP)) > 0) 		
			{
				recvfrom(SOCK_NTP, data_buf, len, (uint8*)&destip, &destport);/*接收NTP服务器回复数据*/
				get_seconds_from_ntp_server(data_buf,startindex);							/*从NTP服务器获取时间*/
				 
//				#if 1
//				sprintf(buf,"%d-%02d-%02d %02d:%02d:%02d",  
//				(nowdate.time.year[0]+nowdate.time.year[1]*256),							
//				nowdate.time.month,
//				nowdate.time.day,
//				nowdate.time.hour,
//				nowdate.time.minute,
//				nowdate.time.second);
//				#endif
//			  
//				#if ERRORLOG
//				ErrorLog(INFO,buf);
//				#endif
        
				if(lastmin != nowdate.time.minute)
       	{			
					lastmin = nowdate.time.minute;
					timepack[4]=(nowdate.time.year[0]+nowdate.time.year[1]*256-2000);
					memcpy(&timepack[5],&nowdate.data[2],5);
					
					check_results=timepack[1];   
					for(i=2;i<(timepack[3]+2+3-1);i++)   //CMD字段到数据字段进行校验
					{
						check_results^=timepack[i];
					}
					timepack[i]=check_results;           //校验值
					timepack[i+1]=0xca;                  //结束符	
					
					#if 0
					printf("timepack:");
					for(i=0;i<sizeof(timepack);i++)d
					{
						printf("%2x ",timepack[i]);
					}
					printf("\r\n");
					#endif
					
					/*发送网络时间数据包至蓝牙识别器*/
					comSendBuf( NRF_PORT , timepack , 12 ); 
			  }					
			}
			break;
		
		/*打开端口*/
		case SOCK_CLOSED: 			
			socket(SOCK_NTP,Sn_MR_UDP,8000,0);
			break;
	}
}
