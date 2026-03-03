/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_subghz_phy.h"
#include <stdbool.h> // for boolean
#include <string.h> // for memcpy
#include <radio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define RF_FREQUENCY 868000000
// #define LORA_SYNCWORD 0x34
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static RadioEvents_t RadioEvents; // structure which contains pointers related to events such as RxDone, TxDone, Timeout and so on
volatile bool DataReceived = false; // force to check memory not cache to avoid misunderstanding of interrupt
volatile uint8_t RxBuffer[255];
volatile uint8_t RxSize = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void OnTxDone(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnTxTimeout(void);
void OnRxTimeout(void);
void OnRxError(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LD1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD1_PORT, &GPIO_InitStruct);
  MX_SubGHz_Phy_Init();
  /* USER CODE BEGIN 2 */
  RadioEvents.RxDone = OnRxDone;
  Radio.Init( &RadioEvents );
  Radio.SetRxConfig(
		  1,                          // Selecting LoRa not GFSK
          0,                          // 125 kHz
          12,                         // Spreading factor SF10 (SF5 to SF12)
          4,                          // 4/8
          0,                          // BandwidthAfc N/A
          12,                         // Preamble Length. bit longer just in case
          0,                          // SymbTimeout continuous RX. will be changed for power consumption
          false,                      // False = Variable length packet
          0,                          // not used because of fixLen
          true,                       // CRC on. Code rate fixes data then CRC checks it's broken or not. Only enable at fixLen = false
          false,                      // Frequency Hopping N/A
          0,                          // Hop Period N/A
          false,                      // Not inverted
          true);                      // Rx continuous mode not single mode
  Radio.SetPublicNetwork(false); // set private network sync word 0x1424
  Radio.SetChannel(RF_FREQUENCY);
  Radio.Rx(0);  //Start receiving data

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    //MX_SubGHz_Phy_Process();

    if(DataReceived == true){

    	if(RxSize > 0)
    	{
    		switch(RxBuffer[0]) // Operating modes
    		{
    		case 0x01:
    			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    			HAL_DELAY(500);
    		    break;
    		case 0x02:
    			break;
    		case 0x03:
    			break;
    		default: // for exceptions
    		}
    	}

    	DataReceived = false;
    }

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Enable HSE Oscillator and Activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_6;
  RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_3;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Set Voltage scale1 as MCU will run at 32MHz */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
	if( size <= 255 )
	{
	 memcpy( RxBuffer, payload, size );
	 RxSize = size;
	}
	DataReceived = true;
    Radio.Rx(0);
}

void OnTxDone(void)
{
}

void OnTxTimeout(void)
{
}

void OnRxTimeout(void)
{
    Radio.Rx(0);
}

void OnRxError(void)
{
    Radio.Rx(0);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
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
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
