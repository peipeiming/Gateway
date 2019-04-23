#include "bsp.h"

/*
*********************************************************************************************************
*	�� �� ��: bsp_InitUartDma
*	����˵��: ���ô��� 3 DMA����
*	��    ��: Memoryaddr,�ڴ��ַ
*         ��Memorysize���ڴ��С
*	�� �� ֵ: ��
*********************************************************************************************************
*/
void bsp_InitUartDma(uint32_t Memoryaddr,uint16_t Memorysize)
{
	DMA_InitTypeDef DMA_InitStructure;
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);					 /*ʹ��DMA����*/
	
  DMA_DeInit( DMA1_Channel3 );  															 /*��DMA��ͨ��3�Ĵ�������Ϊȱʡֵ*/

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART3->DR; /*DMA�������ַ ����3���ݼĴ���*/
	DMA_InitStructure.DMA_MemoryBaseAddr = Memoryaddr;  				 /*DMA�ڴ����ַ*/
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  				 /*���ݴ��䷽�򣬴������ȡ���͵��ڴ�*/
	DMA_InitStructure.DMA_BufferSize = Memorysize;  								 			  /*DMAͨ����DMA����Ĵ�С*/
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  			/*�����ַ�Ĵ�������*/
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  								/*�ڴ��ַ�Ĵ�������*/
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; /*���ݿ��Ϊ8λ*/
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 				/*���ݿ��Ϊ8λ*/
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 											  /*����������ģʽ*/
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 									/*DMAͨ�� xӵ�������ȼ�*/
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  													/*DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��*/

	DMA_Init(DMA1_Channel3, &DMA_InitStructure); 		
	
	DMA_Cmd(DMA1_Channel3, ENABLE);   						/*ʹ��ͨ������*/
	
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); /*ʹ�ܴ���3��DMA����*/
}
