#include "includes.h"

															
/*
*********************************************************************************************************
 * ��������c2i
 * ����  ���ַ�תHexֵ
 * ����  ��
 *       ��p,ASCALLֵָ��
 * ����  : 
 *       ����Ӧ��16����ֵֵ
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
 * ��������app_config_CheckConfig
 * ����  ���������
 * ����  ����
 * ����  : ��

�����ò������ڸ�ʽ˵����
 ����
 1������IP�Ͷ˿ڣ�SETIP=120.79.33.73,PORT=1883; 
 2�������豸MAC�Ϳͻ���ID��SETMAC=0.02.DC.06.AB.CD;
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
		/*�����Ч�ֽ�*/
		if((Receivebuf[i-1]=='\0')&&(NULL==strstr ( temp, "SETIP" )||
			NULL==strstr ( temp, "SETCLIENT" )))  
		{
		  temp=(char *)&Receivebuf[i];
		}
		i%=255;
	}
	
	p=(uint8_t *)strstr ( (const char*)temp, "SETMAC=" );     /*�����豸MAC�Ϳͻ���ID*/
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
 * ��������app_config_start
 * ����  ���ϵ�������
 * ����  ����
 * ����  : ��
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


