#include "includes.h"

/*
*********************************************************************************************************
 * 函数名：ErrorLog
 * 描述  ：设备告警
 * 输入  ：devtype,设备类型
 *       : dev,设备ID
 *       : err_level,告警等级
 *       : err_code,错误码
 *       : err_para_count,错误参数个数
 *       : err_para,错误参数
 * 返回  : 无
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
	
	memset(&topicdata[3],0,2); 														 /*蓝牙网关设备高字节对齐*/
	memcpy(&topicdata[5],sysCfg.parameter.client_mac,6);   /*蓝牙网关设备MAC*/
	
	topicdata[11] = devtype;  			/*设备类型*/
	memcpy(&topicdata[12],dev,8);		/*告警设备ID*/
	topicdata[20] = err_level;      /*告警等级*/
	memcpy(&topicdata[21],(uint8_t *)&err_code,2);  /*错误码*/
	memcpy(&topicdata[23],nowtime.data,7);          /*告警时间*/
	topicdata[30] = err_para_count;
	
	if(err_para_count != 0)  			/*告警参数个数*/
	{
		for(uint8_t i = 0; i < err_para_count; i++)
		{
			para_len += err_para[para_len];
			para_len++;
		}
		memcpy(&topicdata[31],err_para,para_len);
	}
	
	para_len = para_len + 31 + 2; /*有效数据长度*/  
	memcpy(topicdata,(uint8_t *)&para_len,2);
	memcpy(&topicdata[para_len - 2],(uint8_t *)&ErrSerialNumber,4);
	ErrSerialNumber++;
	
	/*CRC校验*/
	crcdata = app_plat_usMBCRC16(topicdata,topicdata[0]+topicdata[1]*256+2);
	memcpy(&topicdata[para_len + 2],(uint8_t *)&crcdata,2);
	
	/*上报数据*/
  mqtt_publish( sysCfg.parameter.data_socket , ALARM_TOPIC , (char *)topicdata , topicdata[0]+topicdata[1]*256 + 4);	
	#endif	
}


