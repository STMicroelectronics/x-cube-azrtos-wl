/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_threadx.c
  * @author  MCD Application Team
  * @brief   ThreadX applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
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
#include "app_threadx.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
TX_THREAD                       MainThread;
TX_SEMAPHORE                    Semaphore;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
VOID MainThread_Entry(ULONG thread_input);
extern void SystemClock_Config(void);
static void SystemClock_Restore(void);
void Enter_LowPower_Mode(void);
void Exit_LowPower_Mode(void);
void App_Delay(ULONG Delay);
/* USER CODE END PFP */

/**
  * @brief  Application ThreadX Initialization.
  * @param memory_ptr: memory pointer
  * @retval int
  */
UINT App_ThreadX_Init(VOID *memory_ptr)
{
  UINT ret = TX_SUCCESS;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL*)memory_ptr;

  /* USER CODE BEGIN App_ThreadX_MEM_POOL */

  /* USER CODE END App_ThreadX_MEM_POOL */

  /* USER CODE BEGIN App_ThreadX_Init */

  CHAR *pointer;

  /* Allocate the stack for MainThread.  */
  if (tx_byte_allocate(byte_pool, (VOID **) &pointer, APP_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    ret = TX_POOL_ERROR;
  }

  /* Create MainThread.  */
  if (tx_thread_create(&MainThread, "Main Thread", MainThread_Entry, 0, pointer, APP_STACK_SIZE, MAIN_THREAD_PRIO,
                       MAIN_THREAD_PREEMPTION_THRESHOLD, TX_NO_TIME_SLICE, TX_AUTO_START) != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* Create a counting semaphore */
  if (tx_semaphore_create(&Semaphore, "idler semaphore",0)  != TX_SUCCESS)
  {
    ret = TX_THREAD_ERROR;
  }

  /* USER CODE END App_ThreadX_Init */

  return ret;
}

/**
  * @brief  App_ThreadX_LowPower_Enter
  * @param  None
  * @retval None
  */
void App_ThreadX_LowPower_Enter(void)
{
  /* USER CODE BEGIN  App_ThreadX_LowPower_Enter */
  Enter_LowPower_Mode();
  /* USER CODE END  App_ThreadX_LowPower_Enter */
}

/**
  * @brief  App_ThreadX_LowPower_Exit
  * @param  None
  * @retval None
  */
void App_ThreadX_LowPower_Exit(void)
{
  /* USER CODE BEGIN  App_ThreadX_LowPower_Exit */
  Exit_LowPower_Mode();
  /* USER CODE END  App_ThreadX_LowPower_Exit */
}

/**
  * @brief  MX_ThreadX_Init
  * @param  None
  * @retval None
  */
void MX_ThreadX_Init(void)
{
  /* USER CODE BEGIN  Before_Kernel_Start */

  /* USER CODE END  Before_Kernel_Start */

  tx_kernel_enter();

  /* USER CODE BEGIN  Kernel_Start_Error */

  /* USER CODE END  Kernel_Start_Error */
}

/* USER CODE BEGIN 1 */

/**
  * @brief  Function implementing the MainThread thread.
  * @param  thread_input: Not used
  * @retval None
  */
void MainThread_Entry(ULONG thread_input)
{
  (void) thread_input;
  UINT count;

  /* Infinite loop */
  while (1)
  {
    if(tx_semaphore_get(&Semaphore, TX_WAIT_FOREVER) == TX_SUCCESS )
    {
      for(count=0; count<10; count++)
      {
        /* Toggle LED to indicate status*/
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_9);
        App_Delay(50);
      }
    }
  }
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  ULONG currentValue = 0;
  if (GPIO_Pin == GPIO_PIN_0)
  {
    tx_semaphore_info_get(&Semaphore, NULL, &currentValue, NULL, NULL, NULL);
    if (currentValue == 0)
    {
      /* Put the semaphore */
      tx_semaphore_put(&Semaphore);
    }
  }
}

/**
  * @brief  This function should be called to enter the low power mode.
  * @param  None
  * @retval None
  */
void Enter_LowPower_Mode(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Turn OFF LED's */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Enable GPIOsclock */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Set all GPIO in analog state to reduce power consumption,                */
  /*   except GPIOA to keep user button interrupt enabled                     */
  /* Note: Debug using ST-Link is not possible during the execution of this   */
  /*       example because communication between ST-link and the device       */
  /*       under test is done through UART. All GPIO pins are disabled (set   */
  /*       to analog input mode) including  UART I/O pins.                    */
  GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();

  /* Enter Stop Mode */
  HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

}

/**
  * @brief  This function should be called to exit the low power mode.
  * @param  None
  * @retval None
  */
void Exit_LowPower_Mode(void)
{
  /* GPIOA Port Clock Enable */
  GPIO_InitTypeDef  GPIO_InitStruct= {0};

  /* Re-configure the system clock to 120 MHz based on MSI, enable and
  select PLL as system clock source (PLL is disabled in STOP mode) */
  SystemClock_Restore();
  /* Enable GPIOB clock */
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Reconfigure GPIO pin : GREEN_LED_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/**
  * @brief  Configures system clock after wake-up from STOP: enable MSI, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
static void SystemClock_Restore(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  uint32_t pFLatency = 0;

  /* Enable MSI oscillator */
  LL_RCC_MSI_Enable();
  while(LL_RCC_MSI_IsReady() != 1)
  {
  };
  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.PLL.PLLState = RCC_MSI_ON;
  while (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
  };

  /* Get the Clocks configuration according to the internal RCC registers */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

  /* Select MSI as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_MSI;
  while (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
  {
  };
}

/**
  * @brief  Application Delay function.
  * @param  Delay : number of ticks to wait
  * @retval None
  */
void App_Delay(ULONG Delay)
{
  ULONG initial_time = tx_time_get();
  while ((tx_time_get() - initial_time) < Delay);
}

/* USER CODE END 1 */
