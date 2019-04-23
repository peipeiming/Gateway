/*
*********************************************************************************************************
*
*	ģ������ : BSPģ��
*	�ļ����� : bsp.h
*	˵    �� : ���ǵײ�����ģ�����е�h�ļ��Ļ����ļ��� Ӧ�ó���ֻ�� #include bsp.h ���ɣ�
*			  ����Ҫ#include ÿ��ģ��� h �ļ�
*
*	Copyright (C), 2013-2014, ���������� www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H

#define SOCK_TCPS         0
#define SOCK_TCP          1
#define SOCK_BLE          2
#define SOCK_DHCP         3
#define SOCK_NTP          4
#define SOCK_HTTP         5

#define MY_MAX_DHCP_RETRY	3
#define DATA_BUF_SIZE   2048
#define STM32_V4
//#define STM32_X2
/* ����Ƿ����˿������ͺ� */
#if !defined (STM32_V4) && !defined (STM32_X2)
	#error "Please define the board model : STM32_X2 or STM32_V4"
#endif

/* ���� BSP �汾�� */
#define __STM32F1_BSP_VERSION		"1.1"

/* CPU����ʱִ�еĺ��� */
//#define CPU_IDLE()		bsp_Idle()

#define  USE_FreeRTOS      1

#if USE_FreeRTOS == 1
	#include "FreeRTOS.h"
	#include "task.h"
	#define DISABLE_INT()    taskENTER_CRITICAL()
	#define ENABLE_INT()     taskEXIT_CRITICAL()
#else
	/* ����ȫ���жϵĺ� */
	#define ENABLE_INT()	__set_PRIMASK(0)	/* ʹ��ȫ���ж� */
	#define DISABLE_INT()	__set_PRIMASK(1)	/* ��ֹȫ���ж� */
#endif

/* ���������ڵ��Խ׶��Ŵ� */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

//#ifndef MCBR03
//	#define MCBR03
//#endif

/* ͨ��ȡ��ע�ͻ�������ע�͵ķ�ʽ�����Ƿ�����ײ�����ģ�� */
#include "bsp_uart_fifo.h"
#include "bsp_led.h"
#include "bsp_timer.h"
#include "spi.h"
#include "timer.h"
#include "bsp_key.h"
#include "bsp_iwdg.h"
#include "bsp_TiMbase.h" 
#include "bsp_mqtt_ota.h"
#include "bsp_cpu_flash.h"
#include "bsp_fsmc_sram.h"

#include "bsp_dma.h"
#include "bsp_sht3x.h"
#include "bsp_sht3x_iic.h"
//#include "bsp_dwt.h"

//#include "bsp_msg.h"

//#include "bsp_beep.h"

//#include "bsp_tim_pwm.h"

//#include "bsp_sdio_sd.h"
//#include "bsp_i2c_gpio.h"
//#include "bsp_eeprom_24xx.h"
//#include "bsp_si4730.h"
//#include "bsp_hmc5883l.h"
//#include "bsp_mpu6050.h"
//#include "bsp_bh1750.h"
//#include "bsp_bmp180.h"
//#include "bsp_wm8978.h"
//#include "bsp_gt811.h"

//#include "bsp_nand_flash.h"
//#include "bsp_nor_flash.h"

//#include "LCD_RA8875.h"
//#include "LCD_SPFD5420.h"
//#include "LCD_ILI9488.h"
//#include "bsp_ra8875_port.h"
//#include "bsp_tft_lcd.h"

//#include "bsp_touch.h"


//#include "bsp_oled.h"
//#include "bsp_sim800.h"
//#include "bsp_ra8875_flash.h"

//#include "bsp_spi_bus.h"
//#include "bsp_spi_flash.h"
//#include "bsp_tm7705.h"
//#include "bsp_vs1053b.h"
//#include "bsp_tsc2046.h"

//#include "bsp_ds18b20.h"
//#include "bsp_dac8501.h"
//#include "bsp_dht11.h"

//#include "bsp_ir_decode.h"
//#include "bsp_ps2.h"

//#include "bsp_modbus.h"
//#include "bsp_rs485_led.h"
//#include "bsp_user_lib.h"

//#include "bsp_dac8501.h"
//#include "bsp_dac8562.h"

//#include "bsp_esp8266.h"
//#include "bsp_step_moto.h"

/* �ṩ������C�ļ����õĺ��� */
void bsp_Init(void);
void bsp_Idle(void);
void BSP_Tick_Init (void);

#endif

/***************************** ���������� www.armfly.com (END OF FILE) *********************************/