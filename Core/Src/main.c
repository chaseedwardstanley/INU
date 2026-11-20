
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ipcc.h"
#include "rf.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define TIM2_COMPARE_INTERVAL_US  1000000U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* Incremented every time TIM2 Channel 1 fires. */
volatile uint32_t tim2CompareCount = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);

/* USER CODE BEGIN PFP */

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

  HAL_Init();

  /* Config code for STM32_WPAN */
  MX_APPE_Config();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();
  PeriphCommonClock_Config();

  MX_IPCC_Init();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_RF_Init();

  /* USER CODE BEGIN 2 */

  /*
   * Start TIM2 as a free-running 32-bit microsecond timer.
   *
   * TIM2 clock: 32 MHz
   * Prescaler: 31
   * Counter frequency: 1 MHz
   * Resolution: 1 microsecond
   */
  if (HAL_TIM_Base_Start(&htim2) != HAL_OK)
  {
      Error_Handler();
  }

  /*
   * Schedule the first Channel 1 compare event
   * one second after the current counter value.
   */
  __HAL_TIM_SET_COMPARE(
      &htim2,
      TIM_CHANNEL_1,
      __HAL_TIM_GET_COUNTER(&htim2) + TIM2_COMPARE_INTERVAL_US
  );

  /*
   * Enable TIM2 Channel 1 output-compare interrupts.
   * No physical output pin is required.
   */
  if (HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1) != HAL_OK)
  {
      Error_Handler();
  }

  /* USER CODE END 2 */

  /* Init code for STM32_WPAN */
  MX_APPE_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    MX_APPE_Process();

    /* USER CODE BEGIN 3 */

    /* USER CODE END 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|
                                   RCC_OSCILLATORTYPE_LSI1|
                                   RCC_OSCILLATORTYPE_HSE|
                                   RCC_OSCILLATORTYPE_MSI;

  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clock dividers.
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|
                                RCC_CLOCKTYPE_HCLK2|
                                RCC_CLOCKTYPE_HCLK|
                                RCC_CLOCKTYPE_SYSCLK|
                                RCC_CLOCKTYPE_PCLK1|
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock.
  */
  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_SMPS | RCC_PERIPHCLK_RFWAKEUP;

  PeriphClkInitStruct.RFWakeUpClockSelection =
      RCC_RFWKPCLKSOURCE_HSE_DIV1024;

  PeriphClkInitStruct.SmpsClockSelection =
      RCC_SMPSCLKSOURCE_HSI;

  PeriphClkInitStruct.SmpsDivSelection =
      RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

/**
  * @brief TIM2 output-compare callback.
  *
  * Called when TIM2 CNT matches CCR1.
  *
  * For this initial validation, schedule an interrupt
  * every 1,000,000 timer ticks (one second).
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if ((htim->Instance == TIM2) &&
        (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
    {
        /*
         * Increment the debugger-visible event counter.
         */
        tim2CompareCount++;

        /*
         * Schedule relative to the previous compare
         * timestamp rather than the current counter.
         *
         * This avoids accumulating interrupt latency
         * into the timer schedule.
         */
        uint32_t nextCompare =
            __HAL_TIM_GET_COMPARE(htim, TIM_CHANNEL_1)
            + TIM2_COMPARE_INTERVAL_US;

        __HAL_TIM_SET_COMPARE(
            htim,
            TIM_CHANNEL_1,
            nextCompare
        );
    }
}

/* USER CODE END 4 */

/**
  * @brief This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

  __disable_irq();

  while (1)
  {
  }

  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief Reports the name of the source file and line number
  *        where the assert_param error has occurred.
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
