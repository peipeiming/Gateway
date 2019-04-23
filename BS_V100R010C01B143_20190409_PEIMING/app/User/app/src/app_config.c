#include "includes.h"

															
/*
*********************************************************************************************************
 * 函数名：c2i
 * 描述  ：字符转Hex值
 * 输入  ：
 *       ：p,ASCALL值指针
 * 返回  : 
 *       ：对应的16进制值值
*********************************************************************************************************
 */
uint8_t c2i(char *p)
{
  uint8_t i;
  uint8_t len;
  uint8_t val=0;

  len=strlen(p);
  for(i=0;i<len;i++)
  {
		if(isdigit(*p))
		{
			val+=(*p-48);
		}
		else if(*p>64&&*p<71)
		{
			val+=(*p-'A');
			val+=10;
		}
		else if(*p>96&&*p<103)
		{
			val+=(*p-'a');
			val+=10;
		}

		if(i==0&&len==2)
		{
			val*=16;
		}
		p++;
  }
  return val;
}

/*
*********************************************************************************************************
 * 函数名：app_config_CheckConfig
 * 描述  ：检查配置
 * 输入  ：无
 * 返回  : 无

【设置参数串口格式说明】
 例：
 1、设置IP和端口：SETIP=120.79.33.73,PORT=1883; 
 2、设置设备MAC和客户端ID：SETMAC=0.02.DC.06.AB.CD;
*********************************************************************************************************
 */
ErrorStatus app_config_CheckConfig(void)
{
	uint8_t i=0,j=0;
	uint8_t *p=NULL;
	uint8_t Receivebuf[256];
	uint8_t IP[4]={0},MAC[6]={0};
	
	char *temp=NULL;
	char bcdbuf[3],tem;
	
	uint16_t port=0;
	
	temp=(char *)Receivebuf;	
	memset(Receivebuf,0,256);

	while(comGetChar(COM1,&Receivebuf[i++]))
  {
		/*规避无效字节*/
		if((Receivebuf[i-1]=='\0')&&(NULL==strstr ( temp, "SETIP" )||
			NULL==strstr ( temp, "SETCLIENT" )))  
		{
		  temp=(char *)&Receivebuf[i];
		}
		i%=255;
	}
	
	p=(uint8_t *)strstr ( (const char*)temp, "SETMAC=" );     /*设置设备MAC和客户端ID*/
	if(NULL!=p)
	{
		p+=7;
		for(i=0;j<6;i++)
		{
			if(p[i]!='.')
			{
				tem=p[i];
				bcdbuf[0]=p[i];
				if(p[i+1]!='.')
			  {
					bcdbuf[1]=p[++i];
					bcdbuf[2]='\0';
					MAC[j++]=c2i(bcdbuf);
				}
				else
				{
					MAC[j++]=c2i(&tem);
				}
			}
		}
	
		if(SUCCESS==app_flash_SetDeviceClient(MAC))
	  {
			printf("set client mac ok.\r\n");
			delay_ms(1000);
			NVIC_SystemReset();			
		}
	}	
	
	return ERROR;
}

/*
*********************************************************************************************************
 * 函数名：app_config_start
 * 描述  ：上电检查配置
 * 输入  ：无
 * 返回  : 无
*********************************************************************************************************
 */
void app_config_start(void)
{
	uint8_t i=5;
	
	printf("Please set config parameter....");
	while(i>0)
	{
	  printf("%d ",i);
		delay_ms(1000);
		app_config_CheckConfig();
		i--;
	}
	printf("\r\n");
}


