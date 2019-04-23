#include "includes.h"

/*
*********************************************************************************************************
 * ��������ErrorLog
 * ����  ���豸�澯
 * ����  ��devtype,�豸����
 *       : dev,�豸ID
 *       : err_level,�澯�ȼ�
 *       : err_code,������
 *       : err_para_count,�����������
 *       : err_para,�������
 * ����  : ��
*********************************************************************************************************
*/
void ErrorLog(uint8_t devtype,uint8_t *dev,uint8_t err_level,uint16_t err_code,uint8_t err_para_count,uint8_t *err_para)
{	
	#if ERRORLOG
  DateTime nowtime;
	uint16_t crcdata = 0;
	uint16_t para_len = 0;
	uint8_t topicdata[256] = {0x00,0x00,0x64};
	
	static uint32_t ErrSerialNumber = 0;

	get_ntp_time(&nowtime);
	
	memset(&topicdata[3],0,2); 														 /*���������豸���ֽڶ���*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);   /*���������豸MAC*/
	
	topicdata[11] = devtype;  			/*�豸����*/
	memcpy(&topicdata[12],dev,8);		/*�澯�豸ID*/
	topicdata[20] = err_level;      /*�澯�ȼ�*/
	memcpy(&topicdata[21],(uint8_t *)&err_code,2);  /*������*/
	memcpy(&topicdata[23],nowtime.data,7);          /*�澯ʱ��*/
	topicdata[30] = err_para_count;
	
	if(err_para_count != 0)  			/*�澯��������*/
	{
		for(uint8_t i = 0; i < err_para_count; i++)
		{
			para_len += err_para[para_len];
			para_len++;
		}
		memcpy(&topicdata[31],err_para,para_len);
	}
	
	para_len = para_len + 31 + 2; /*��Ч���ݳ���*/  
	memcpy(topicdata,(uint8_t *)&para_len,2);
	memcpy(&topicdata[para_len - 2],(uint8_t *)&ErrSerialNumber,4);
	ErrSerialNumber++;
	
	/*CRCУ��*/
	crcdata = app_plat_usMBCRC16(topicdata,topicdata[0]+topicdata[1]*256+2);
	memcpy(&topicdata[para_len + 2],(uint8_t *)&crcdata,2);
	
	/*�ϱ�����*/
  mqtt_publish( sysCfg.parameter.data_socket , ALARM_TOPIC , (char *)topicdata , topicdata[0]+topicdata[1]*256 + 4);	
	#endif	
}


