/*MCU与W5500通信：SPI接口，采用其它SPI组，需修改相信的GPIO口*/
#include "bsp.h"
/**
  * @brief  使能SPI时钟
  * @retval None
  */
static void SPI_RCC_Configuration(void)
{
	#ifdef MCBR03
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);   //使能SPI3时钟 
	#else
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);   //使能SPI1时钟 
	#endif
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);   //使能复用时钟，管脚复用，管脚在APB2上
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}
/**
  * @brief  配置指定SPI的引脚
  * @retval None
  */
static void SPI_GPIO_Configuration(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	
	#ifdef MCBR03
	//PA15->CS,PB3->SCK,PB4->MISO,PB5->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	
	//初始化片选输出引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);
	#else
	//PB12->CS,PB13->SCK,PB14->MISO,PB15->MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure); 	
	//初始化片选输出引脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
	#endif
}
/**
  * @brief  根据外部SPI设备配置SPI相关参数
  * @retval None
  */
void SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStruct;

	SPI_RCC_Configuration();
	SPI_GPIO_Configuration();
	
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStruct.SPI_Direction= SPI_Direction_2Lines_FullDuplex;
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStruct.SPI_CRCPolynomial = 7;

	#ifdef MCBR03
	SPI_Init(SPI3,&SPI_InitStruct);
	#else	
	SPI_Init(SPI2,&SPI_InitStruct);
	#endif
	
	#ifdef MCBR03
	SPI_Cmd(SPI3, ENABLE);
	#else
	SPI_Cmd(SPI2, ENABLE);
	#endif
}
/**
  * @brief  写1字节数据到SPI总线
  * @param  TxData 写到总线的数据
  * @retval None
  */
void SPI_WriteByte(uint8_t TxData)
{				
	#ifdef MCBR03
	while((SPI3->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空		  
	SPI3->DR=TxData;	 	  									//发送一个byte 
	while((SPI3->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	SPI3->DR;	
	#else	
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空		  
	SPI2->DR=TxData;	 	  									//发送一个byte 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	SPI2->DR;	
  #endif	
}
/**
  * @brief  从SPI总线读取1字节数据
  * @retval 读到的数据
  */
uint8_t SPI_ReadByte(void)
{			 
	#ifdef MCBR03
	while((SPI3->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空			  
	SPI3->DR=0xFF;	 	  										//发送一个空数据产生输入数据的时钟 
	while((SPI3->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	return SPI3->DR;  
	#else
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==0);	//等待发送区空			  
	SPI2->DR=0xFF;	 	  										//发送一个空数据产生输入数据的时钟 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==0); //等待接收完一个byte  
	return SPI2->DR;  						    
	#endif
}
/**
  * @brief  进入临界区
  * @retval None
  */
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);
}
/**
  * @brief  退出临界区
  * @retval None
  */
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);
}

/**
  * @brief  片选信号输出低电平
  * @retval None
  */
void SPI_CS_Select(void)
{
	#ifdef MCBR03
	GPIO_ResetBits(GPIOA,GPIO_Pin_15);
	#else
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	#endif
}
/**
  * @brief  片选信号输出高电平
  * @retval None
  */
void SPI_CS_Deselect(void)
{
	#ifdef MCBR03
	GPIO_SetBits(GPIOA,GPIO_Pin_15);
	#else
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	#endif
}
/*********************************END OF FILE**********************************/

