#include "bsp.h"

/*
*********************************************************************************************************
*	函 数 名: bsp_InitUartDma
*	功能说明: 配置串口 3 DMA传输
*	形    参: Memoryaddr,内存地址
*         ：Memorysize，内存大小
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitUartDma(uint32_t Memoryaddr,uint16_t Memorysize)
{
	DMA_InitTypeDef DMA_InitStructure;
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);					 /*使能DMA传输*/
	
  DMA_DeInit( DMA1_Channel3 );  															 /*将DMA的通道3寄存器重设为缺省值*/

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; /*DMA外设基地址 串口3数据寄存器*/
	DMA_InitStructure.DMA_MemoryBaseAddr = Memoryaddr;  				 /*DMA内存基地址*/
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  				 /*数据传输方向，从外设读取发送到内存*/
	DMA_InitStructure.DMA_BufferSize = Memorysize;  								 			  /*DMA通道的DMA缓存的大小*/
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  			/*外设地址寄存器不变*/
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  								/*内存地址寄存器递增*/
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /*数据宽度为8位*/
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 				/*数据宽度为8位*/
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 											  /*工作在连续模式*/
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 									/*DMA通道 x拥有中优先级*/
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  													/*DMA通道x没有设置为内存到内存传输*/

	DMA_Init(DMA1_Channel3, &DMA_InitStructure); 		
	
	DMA_Cmd(DMA1_Channel3, ENABLE);   						/*使能通道传输*/
	
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); /*使能串口3的DMA发送*/
}
