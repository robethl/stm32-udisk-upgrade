
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_diskio_dma.h"
#include "usart.h"
#include "gpio.h"
#include "typedef.h"
#include "lcd.h"
#include "db.h"

/* Private define ------------------------------------------------------------*/
typedef enum {
  APPLICATION_IDLE = 0,
  APPLICATION_READY,
}MSC_ApplicationTypeDef;

/* Private macro -------------------------------------------------------------*/
#define OTA_VER_P	1
#define OTA_VER_S	2

/* Private variables ---------------------------------------------------------*/
USBH_HandleTypeDef hUSB_Host;
MSC_ApplicationTypeDef Appli_state = APPLICATION_IDLE;
char USBDISKPath[4];            /* USB Host logical drive path */

/**
  * @brief System Clock Configuration
  * @retval None
  */
static void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/**Configure the main internal regulator output voltage 
	*/
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 6;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
								|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
	_Error_Handler(__FILE__, __LINE__);
	}

	/**Enables the Clock Security System 
	*/
	HAL_RCC_EnableCSS();

	/**Configure the Systick interrupt time 
	*/
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick 
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while(1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

static bool timespanExpired(uint32_t time, uint32_t timespan_ms)
{
	uint32_t timespan_actual = 0;

	uint32_t cur_time = HAL_GetTick();
	if(time > cur_time){
		timespan_actual = 0xffffffff - (time - cur_time);
	}else{
		timespan_actual = cur_time - time;
	}

	if(timespan_actual > timespan_ms)return true;
	return false;
}

static uint32_t timespanStart(void)
{
	return HAL_GetTick();
}

/**
  * @brief  User Process
  * @param  phost: Host Handle
  * @param  id: Host Library user message ID
  * @retval None
  */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{ 
	switch(id)
	{
		case HOST_USER_SELECT_CONFIGURATION:
			break;

		case HOST_USER_DISCONNECTION:
			Appli_state = APPLICATION_IDLE;
			if (f_mount(NULL, "", 0) != FR_OK){
				printf("ERROR : Cannot DeInitialize FatFs! \n");
			}
			if (FATFS_UnLinkDriver(USBDISKPath) != 0){
				printf("ERROR : Cannot UnLink USB FatFS Driver! \n");
			}
			break;

		case HOST_USER_CLASS_ACTIVE:
			Appli_state = APPLICATION_READY;
			break;

		case HOST_USER_CONNECTION:
			if (FATFS_LinkDriver(&USBH_Driver, USBDISKPath) == 0){
				static FATFS USBH_fatfs;
				if (f_mount(&USBH_fatfs, "", 0) != FR_OK){
					printf("ERROR : Cannot Initialize FatFs! \n");
					lcdDisplay(LINE_ONE, "ERROR");
				}
			}
			break;

		default:
			break;
	}
}

void ledAlarm(bool on)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void ledStatus(bool on)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, on ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void systemInit(void)
{
	HAL_Init();	
	SystemClock_Config();

	MX_USART1_UART_Init();
	MX_GPIO_Init();
	lcdInit();
	lcdSetBkled(BKLED_LIGHT);
	lcdDisplay(LINE_ONE, "Boot");
	lcdDisplay(LINE_TWO, "scan usb...");

	USBH_Init(&hUSB_Host, USBH_UserProcess, 0);
	USBH_RegisterClass(&hUSB_Host, USBH_MSC_CLASS);
	USBH_Start(&hUSB_Host);
}

static void jumpToApp(void)
{
	#define APP_ADDR 0x08010000 //应用程序首地址定义 
	typedef void (*APP_FUNC)(); //函数指针类型定义

  APP_FUNC jump2app;

	HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

	 /* 栈顶地址是否合法(sram 64k)*/
    if(((*(__IO uint32_t *)APP_ADDR) & 0x2FFF0000) == 0x20000000){
		printf("Run application!\n\n");
        __set_MSP(APP_ADDR);  /* 设置栈指针 */
        jump2app = ( APP_FUNC )(*(__IO uint32_t *)(APP_ADDR + 4)); /* 复位*/
        jump2app();
    } 

    printf("application isn't found!\n");
}

int main(void)
{
	int32_t ret = 0;
	uint32_t timeout_tick = 0;
	systemInit();
	printf("+++++++++++++++++++++++++++++++++++++++++\n");
	printf("+            STM32 bootloader           +\n");
	printf("+             v%d.%02d usb OTG             +\n", OTA_VER_P, OTA_VER_S);
	printf("+++++++++++++++++++++++++++++++++++++++++\n");
	dbLoad();
	timeout_tick = timespanStart();
	while (1){
		USBH_Process(&hUSB_Host);
		if(APPLICATION_READY == Appli_state){
			extern bool bootCheckAppVersion(void);
			extern int32_t bootOta(uint32_t add);
			timeout_tick = timespanStart();
			if(bootCheckAppVersion()){
				ledStatus(true);
				ret = bootOta(APP_ADDR);
				if(0 == ret){
					printf("usb upgrade(%dms)\n", HAL_GetTick() - timeout_tick);
					ledStatus(false);
				}
			}else{
				ledAlarm(true);
			}
			break;
		}
		
		if(timespanExpired(timeout_tick, 2000)){
			printf("no usb device connect\n");
			break;
		}
	}

	if(-2 == ret){
		lcdDisplayCenter(LINE_TWO, "upgrade fail");
		ledAlarm(true);
	}else{
		//if(dbCheckAppValid()){
			lcdDisplayCenter(LINE_TWO, "jump to app");
			jumpToApp();
		//} else {
			ledAlarm(true);
			lcdDisplay(LINE_ONE, "[Error]");
			lcdDisplayCenter(LINE_TWO, "no app");
		//}
	}
}
