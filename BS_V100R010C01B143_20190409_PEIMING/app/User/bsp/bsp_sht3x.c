#include "bsp.h"

unsigned char buffer[6];
void delay_us(uint32_t time)
{
	uint32_t i=0;
	for(i=0;i<time;i++){
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
		__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();__NOP();
	}

}

void SHT3X_WriteCMD(unsigned int cmd)
{
  IIC_Start();
  IIC_Send_Byte(i2cAddWrite_8bit);
  IIC_Wait_Ack();
  IIC_Send_Byte(cmd>>8);
  IIC_Wait_Ack();
  IIC_Send_Byte(cmd);
  IIC_Wait_Ack();
  IIC_Stop();
}

//读取SHT30寄存器状态
void SHT3X_ReadState(unsigned char *temp)
{
    IIC_Start();
    IIC_Send_Byte(i2cAddWrite_8bit);
    IIC_Wait_Ack();
    IIC_Send_Byte(0xf3);
    IIC_Wait_Ack();
    IIC_Send_Byte(0X2d);
    IIC_Wait_Ack();
  
    IIC_Start();
    IIC_Send_Byte(i2cAddRead_8bit);
    IIC_Wait_Ack();

    temp[0] = IIC_Read_Byte(1);//高
    temp[1] = IIC_Read_Byte(1);//低
    temp[2] = IIC_Read_Byte(0);//校验
    IIC_Stop(); 
}

void SHT3X_Init(void)
{
  delay_us(250);
  SHT3X_WriteCMD(CMD_MEAS_PERI_2_H);//设置测量周期和模式
}

//读取SHT30结果
void SHX3X_ReadResults(unsigned int cmd,  unsigned char *p)
{
	IIC_Start();
	IIC_Send_Byte(i2cAddWrite_8bit);
	IIC_Wait_Ack();
	IIC_Send_Byte(cmd>>8);
	IIC_Wait_Ack();
	IIC_Send_Byte(cmd);
	IIC_Wait_Ack();

	IIC_Start();
	IIC_Send_Byte(i2cAddRead_8bit);

	if(IIC_Wait_Ack()==0)
	{     
		p[0] = IIC_Read_Byte(1);//温度高
		p[1] = IIC_Read_Byte(1);//温度低
		p[2] = IIC_Read_Byte(1);//校验
		p[3] = IIC_Read_Byte(1);//湿度高
		p[4] = IIC_Read_Byte(1);//湿度低
		p[5] = IIC_Read_Byte(0);//校验
		IIC_Stop();
	}
}

//校验
unsigned char SHT3X_CalcCrc(unsigned char *data, unsigned char nbrOfBytes)
{
	unsigned char bit;        // bit mask
    unsigned char crc = 0xFF; // calculated checksum
    unsigned char byteCtr;    // byte counter

    // calculates 8-Bit checksum with given polynomial
    for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) 
	{
        crc ^= (data[byteCtr]);
        for(bit = 8; bit > 0; --bit) 
		{
            if(crc & 0x80) 
				crc = (crc << 1) ^ POLYNOMIAL;
			else 
                crc = (crc << 1);
        }
    }
	return crc;
}

//校验检测
unsigned char SHT3X_CheckCrc(unsigned char *pdata, unsigned char nbrOfBytes, unsigned char checksum)
{
    unsigned char crc;
	crc = SHT3X_CalcCrc(pdata, nbrOfBytes);// calculates 8-Bit checksum
    if(crc != checksum) 
        return 1;           
    return 0;              
}

//计算温度
unsigned int SHT3X_CalcTemperature(unsigned int rawValue)
{
    // calculate temperature 
    unsigned int temp;
    temp = (175 * (float)rawValue / 65535 - 45) ; // T = -45 + 175 * rawValue / (2^16-1)
    return temp;
}

//计算湿度
unsigned char SHT3X_CalcRH(unsigned int rawValue)
{
    // calculate relative humidity [%RH]
    unsigned char temp1 = (100 * (float)rawValue / 65535) ;  // RH = rawValue / (2^16-1) * 10
    return temp1;
}

//读取温度---20ms即可读取完成
void SHT_GetValue(unsigned char *TemValue, unsigned char *RhValue)
{
    unsigned char temp = 0;
    unsigned int dat;
    unsigned char p[3];
    unsigned char cnt;
    unsigned char tem_status,hum_status;
    cnt = 0;
    tem_status = 0;
    hum_status = 0;
    
    while(cnt++<2)
	{
		SHX3X_ReadResults(CMD_FETCH_DATA, buffer);//获取温度湿度

		p[0] = buffer[0];
		p[1] = buffer[1];
		p[2] = buffer[2];
		temp = SHT3X_CheckCrc(p,2,p[2]);//校验
		if( !temp ) 
		{
			dat = ((unsigned int)buffer[0] << 8) | buffer[1];
			*TemValue = SHT3X_CalcTemperature( dat )-TEM_CHEAK_VALUE;    //计算温度加校正
			tem_status = 0;
		}
		else
			tem_status = 1;  

		p[0] = buffer[3];
		p[1] = buffer[4];
		p[2] = buffer[5];
		temp = SHT3X_CheckCrc(p,2,p[2]);//校验
		if( !temp )
		{
			dat = ((unsigned int)p[0] << 8) | p[1];
			*RhValue = SHT3X_CalcRH( dat )+RH_CHEAK_VALUE; //计算湿度加校正
			hum_status = 0;
		}
		else
			hum_status = 1;  

		if((tem_status==0) && (hum_status==0))  
			break;
		
		delay_us(10); 
	}
	
	if((tem_status!=0) || (hum_status!=0))
	{   
		//      SCL_OUT();
		delay_us(250);
		SHT3X_WriteCMD(CMD_MEAS_PERI_2_H);//设置测量周期和模式
		delay_us(150);
	}
}
/*
*/

