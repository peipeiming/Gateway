/*******************************************************************************
** �ļ���: 		mian.c
** �汾��  		1.0
** ��������: 	RealView MDK-ARM 4.14
** ����: 		wuguoyana
** ��������: 	2011-04-28
** ����:		USART��ʼ����RCC���ã�Ȼ���common.c��ִ�����˵�
** ����ļ�:	stm32f10x.h
** �޸���־��	2011-04-29   �����ĵ�
*******************************************************************************/
/* ����ͷ�ļ� *****************************************************************/
#include "common.h"
/* �������� ------------------------------------------------------------------*/
/* �� ------------------------------------------------------------------------*/
#define LED2   GPIO_Pin_6
#define LED3   GPIO_Pin_7
#define LED4   GPIO_Pin_8
#define LED5   GPIO_Pin_9

#define FLAGADRR   0x0807F000    /*��־λ����ַ*/

/* ���� ----------------------------------------------------------------------*/
extern pFunction Jump_To_Application;
extern uint32_t JumpAddress;

/* �������� ------------------------------------------------------------------*/
void Delay(__IO uint32_t nCount);
void LED_Configuration(void);
static void IAP_Init(void);
void KEY_Configuration(void);
void GPIO_Configuration(void);
void USART_Configuration(void);
/* �������� ------------------------------------------------------------------*/

/*******************************************************************************
  * @��������	main
  * @����˵��   ������
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
int main(void)
{
	   uint32_t fw_len=0xFFFFFFFF;
		 uint8_t value=5;
		 uint16_t i=0;
	   uint16_t data=0;
     uint32_t nPage;
	   uint32_t nErasedPage;
    //Flash ����
    FLASH_Unlock();
    IAP_Init();
	  bsp_InitUart(); /* ��ʼ������ */
    if(!(bsp_ReadCpuFlash(FLAGADRR,(uint8_t *)&fw_len,4)))
		{
			SerialPutString("\r\nEnter Bootloader Sucessful\r\n");
		}
		else
		{
			//SerialPutString("\r\nEnter Bootloader Fail\r\n");
		}
		if (fw_len!=0xFFFFFFFF)
			{	
          //printf("\r\fw_len:%d\r\n",fw_len);
				  //SerialPutString("\r\nReady to update\r\n");
					nPage=FLASH_PagesMask(fw_len);
					// Clear All pending flags
					FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
					//erase required pages
					for(nErasedPage=0; nErasedPage<nPage; nErasedPage++)
					{
						FLASH_ErasePage(ApplicationAddress + 0x800*nErasedPage);
					}
					//copy app from backup address to real address
					for(i=0; i<fw_len; i+=2)
					{
						FLASH_ProgramHalfWord(ApplicationAddress+i, *(uint16_t*)(AppBackupAddress+i));

						//printf("addr:%lx\r\n",ApplicationAddress+i);
						//printf("data:%lx\r\n",*(__IO uint16_t*)(ApplicationAddress+i));
					}
//					//����backup flash
//					for(nErasedPage=0; nErasedPage<nPage; nErasedPage++)
//					{
//						FLASH_ErasePage(AppBackupAddress + 0x800*nErasedPage);
//					}
//					
					//lock flash again
					FLASH_Lock();
				  if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
          {
            //SerialPutString("enter user Program\r\n");
            //��ת���û�����
            JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;

            //��ʼ���û�����Ķ�ջָ��
            __set_MSP(*(__IO uint32_t*) ApplicationAddress);
            Jump_To_Application(); //ִ���û�����
        }
			}
   
    else
    {
        //�ж��û��Ƿ��Ѿ����س�����Ϊ��������´˵�ַ��ջ��ַ��
        //��û����һ��Ļ�����ʹû�����س���Ҳ�����������ܷɡ�
        if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000)
        {
            //SerialPutString("\r\nEnter user Program\r\n");
            //��ת���û�����
            JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);
            Jump_To_Application = (pFunction) JumpAddress;

            //��ʼ���û�����Ķ�ջָ��
            __set_MSP(*(__IO uint32_t*) ApplicationAddress);
            Jump_To_Application(); //ִ���û�����
        }
    }

    while (1)
    {
    }
}


/*******************************************************************************
  * @��������	LED_Configuration
  * @����˵��   ����ʹ��LED
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void LED_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //ʹ��LED����GPIO��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);
    //��ʼ��LED��GPIO
    GPIO_InitStructure.GPIO_Pin = LED2 | LED3 | LED4 | LED5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,LED2 | LED3 | LED4 | LED5);  //Ϩ��LED2-5
}

/*******************************************************************************
  * @��������	KEY_Configuration
  * @����˵��   ������ʼ��
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void KEY_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    //���ð���
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
  * @��������	GPIO_Configuration
  * @����˵��   ����ʹ��USART1�����IO�ܽ�
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    // ���� USART1 Tx (PA.09) ��Ϊ�������Ų��������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //���� USART1 Tx (PA.10) ��Ϊ�������Ų��Ǹ�������ģʽ
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************************
  * @��������	IAP_Init
  * @����˵��   ����ʹ��IAP
  * @�������   ��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void IAP_Init(void)
{
    USART_InitTypeDef USART_InitStructure;

    /* USART1 ���� ------------------------------------------------------------
         USART1 ��������:
          - ������      = 115200 baud
          - �ֳ�        = 8 Bits
          - һ��ֹͣλ
          - ��У��
          - ��Ӳ��������
          - ���ܺͷ���ʹ��
    --------------------------------------------------------------------------*/
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    GPIO_Configuration();
    USART_Init(USART1, &USART_InitStructure);
    // ʹ�� USART1
    USART_Cmd(USART1, ENABLE);
}

/*******************************************************************************
  * @��������	Delay
  * @����˵��   ����һ����ʱʱ��
  * @�������   nCount: ָ����ʱʱ�䳤��
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void Delay(__IO uint32_t nCount)
{
    for (; nCount != 0; nCount--);
}

#ifdef  USE_FULL_ASSERT

/*******************************************************************************
  * @��������	assert_failed
  * @����˵��   �����ڼ�������������ʱ��Դ�ļ����ʹ�������
  * @�������   file: Դ�ļ���
  				line: ������������
  * @�������   ��
  * @���ز���   ��
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* �û����������Լ��Ĵ������ڱ��������ļ�������������,
       ���磺SerialPutString("�������ֵ: �ļ��� %s �� %d��\r\n", file, line) */

    //��ѭ��
    while (1)
    {
    }
}
#endif

/*******************************�ļ�����***************************************/