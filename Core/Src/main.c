/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CRC_POLYNOMIAL 0x1D
#define CRC_SEED 0xAA

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma location=0x2004c000
ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
#pragma location=0x2004c0a0
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __CC_ARM )  /* MDK ARM Compiler */

__attribute__((at(0x2004c000))) ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT]; /* Ethernet Rx DMA Descriptors */
__attribute__((at(0x2004c0a0))) ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT]; /* Ethernet Tx DMA Descriptors */

#elif defined ( __GNUC__ ) /* GNU Compiler */

ETH_DMADescTypeDef DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDecripSection"))); /* Ethernet Rx DMA Descriptors */
ETH_DMADescTypeDef DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDecripSection")));   /* Ethernet Tx DMA Descriptors */
#endif

ETH_TxPacketConfig TxConfig;

ADC_HandleTypeDef hadc3;

ETH_HandleTypeDef heth;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart3;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ETH_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_USB_OTG_FS_PCD_Init(void);
static void MX_ADC3_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t crc_value = 0;
uint8_t checkmark = 0;
uint8_t bit_counter = 0;
uint8_t word_counter = 0;
uint8_t reg_counter = 0;
uint8_t reg_selector = 0;
uint8_t sequence_counter = 0;
uint16_t vprog_counter = 0;

uint16_t picker = 1;

const uint8_t FrameMatrix[2][6] = {{0, 1  ,1,    0,     1,   1}, {0     ,0     ,1,     0   ,1    ,1 }};
const uint8_t TIFEN_Matrix[6] = {0,0,0,0,1,1};

const uint16_t WordABCD = 0xABCD;
uint16_t EPROMreg[2][18] = {0}; // you can read EEPROM in 'live expressions' sesction in debugging. Just run the program, go into debug section (shortcut: F11), click 'resume' (F8) and go into 'live expressions'.
const uint16_t ReadWords[38] = {0x8250, 0x8000, 0x0400, 0xffff, 0x0410, 0xffff, 0x0420, 0xffff, 0x0430, 0xffff, 0x0440, 0xffff,
		 0x0450, 0xffff,  0x0460, 0xffff, 0x0470, 0xffff, 0x0480, 0xffff, 0x0490, 0xffff, 0x04a0, 0xffff, 0x04b0, 0xffff,
		 0x04c0, 0xffff, 0x04d0, 0xffff, 0x04e0, 0xffff, 0x04f0, 0xffff, 0x0500, 0xffff, 0x0510, 0xffff};
//uint16_t WriteWords[18] = {1,2,3,4, 5,6,7,8, 9,0xa,0xb,0xc, 0xd,0xe,0xf,0x10, 0x11,0x12}; // dodac wyrazy do zapisania na EEPROM
uint16_t ProgrammingSeq[12] = {0x8400, 0x0, 0x83e0, 0x024f, 0x83e0, 0x024c, 0x8400, 0x0, 0x83e0, 0x024e, 0x83e0, 0x024c}; // dodac programming sequence
uint8_t glosnik_counter = 0;
//Below is the same values as default but with 0xc00c on 0x40 address which means that MEAS_RNG is 50A with 24mV/A
uint16_t WriteWords[18] = {0xc038,0x8e57,0x0,0xf288,0x16f,0x1,0xc862,0xe1fb,0xff,0xe6,0x3e,0x18e7,0x0,0xb700,0xc0f,0x1c51,0xb6cf,0x180}; // dodac wyrazy do zapisania na EEPROM

//variables for custom message:

const uint16_t CustomMsg[] = {0x8250, 0x8000, 0x0180, 0xffff};//custom massage to send to TLI4971
#define custom_msg_size (sizeof(CustomMsg)/sizeof(CustomMsg[0]))
const uint8_t AnswerSchematic[custom_msg_size] = {0, 0, 0, 1};// Most of the time you don't need to read what TLI is sending so to not fill AnswerCustomArray with garbage, you can write '1' to specify when to read the answer. Write all '1' in array to read every word the TLI is sending.
uint8_t AnswerCustomMsg_counter = 0;
uint16_t AnswerCustomMsg[custom_msg_size] = {0};
//uint16_t *AnswerCustomMsg = NULL;



//uint32_t WORD[5] = {0xabcd, 0x8250, 0x8000, 0x180, 0xffff}; // TEMPERATURA;
//                    0         1      2      3        4        5     6        7      8       9      10        11
//uint32_t WORD[39] = {0xabcd, 0x8250, 0x8000, 0x0400,0xFFFF, 0x0410,0xFFFF,0x0420,0xFFFF,0x0430,0xFFFF,0x0440,0xFFFF,0x0450,0xFFFF,0x0460,0xFFFF,0x0470,0xFFFF,0x0480,0xFFFF,0x0490,0xFFFF,0x04a0,0xFFFF,0x04b0,0xFFFF,0x04c0,0xFFFF,0x04d0,0xFFFF,0x04e0,0xFFFF,0x04f0,0xFFFF,0x0500,0xFFFF,0x0510,0xFFFF};
//uint32_t WORD[12] = {0xabcd, 0x8250, 0x8000, 0x8010, 0x0000, 0x0110,0xFFFF, 0x8110, 0xc00c, 0xffff, 0x0110, 0xffff };
//uint32_t WORD[10] = {0xabcd, 0x8250, 0x8000, 0x8010, 0x0000, 0x0420, 0x0420, 0x0410, 0x0410, 0xffff};
//uint32_t WORD[12] = {0xabcd, 0x8250, 0x8000,0x8000, 0x8010, 0x0400, 0xffff, 0x8400, 0xc00c, 0xffff, 0x0400, 0xffff }; //, 0x8010, 0x0000, 0x0430,0xFFFF, 0x0140, 0xFFFF };

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

//	AnswerCustomMsg = calloc(sumOfArray(AnswerSchematic, sizeof(AnswerSchematic)), sizeof(uint16_t));
//	AnswerCustomMsg = calloc( 4, sizeof(uint16_t));

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
  MX_GPIO_Init();
  MX_ETH_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_ADC3_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim1);
  HAL_TIM_Base_Start_IT(&htim4);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief ETH Initialization Function
  * @param None
  * @retval None
  */
static void MX_ETH_Init(void)
{

  /* USER CODE BEGIN ETH_Init 0 */

  /* USER CODE END ETH_Init 0 */

   static uint8_t MACAddr[6];

  /* USER CODE BEGIN ETH_Init 1 */

  /* USER CODE END ETH_Init 1 */
  heth.Instance = ETH;
  MACAddr[0] = 0x00;
  MACAddr[1] = 0x80;
  MACAddr[2] = 0xE1;
  MACAddr[3] = 0x00;
  MACAddr[4] = 0x00;
  MACAddr[5] = 0x00;
  heth.Init.MACAddr = &MACAddr[0];
  heth.Init.MediaInterface = HAL_ETH_RMII_MODE;
  heth.Init.TxDesc = DMATxDscrTab;
  heth.Init.RxDesc = DMARxDscrTab;
  heth.Init.RxBuffLen = 1524;

  /* USER CODE BEGIN MACADDRESS */

  /* USER CODE END MACADDRESS */

  if (HAL_ETH_Init(&heth) != HAL_OK)
  {
    Error_Handler();
  }

  memset(&TxConfig, 0 , sizeof(ETH_TxPacketConfig));
  TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
  TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
  TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;
  /* USER CODE BEGIN ETH_Init 2 */

  /* USER CODE END ETH_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00808CD2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 720;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 720-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 10-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 7200-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief USB_OTG_FS Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_OTG_FS_PCD_Init(void)
{

  /* USER CODE BEGIN USB_OTG_FS_Init 0 */

  /* USER CODE END USB_OTG_FS_Init 0 */

  /* USER CODE BEGIN USB_OTG_FS_Init 1 */

  /* USER CODE END USB_OTG_FS_Init 1 */
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 6;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = ENABLE;
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_OTG_FS_Init 2 */

  /* USER CODE END USB_OTG_FS_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, AOUT_Pin|glosnik_output_Pin|VPROG_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GATE2old_Pin|USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AOUT_Pin */
  GPIO_InitStruct.Pin = AOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(AOUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : glosnik_output_Pin VPROG_Pin */
  GPIO_InitStruct.Pin = glosnik_output_Pin|VPROG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : GATE2old_Pin USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = GATE2old_Pin|USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : ANSWER_Pin */
  GPIO_InitStruct.Pin = ANSWER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ANSWER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : POWER_Pin */
  GPIO_InitStruct.Pin = POWER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(POWER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : TRIGGER_Pin */
  GPIO_InitStruct.Pin = TRIGGER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(TRIGGER_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
uint8_t crc8(uint8_t *data, uint8_t length) {
    uint32_t crc = CRC_SEED;

    for (int i = 0; i < length; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc <<= 1;
                crc ^= CRC_POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
    }
    return (uint8_t)(~crc);
}

uint8_t crcCalc(uint16_t *data, int len) {
    uint8_t crcData8[len * 2];

    for (int i = 0; i < len; i++) {
        crcData8[i * 2]     = (data[(i + 3) % 18] >> 8 ) & 0xFF; // high byte
        crcData8[i * 2 + 1] = (data[(i + 3) % 18]) & 0xFF;      // low byte
    }

    return crc8(crcData8, len * 2 - 1); // exclude last byte
}

bool checkCRC(uint16_t *data, int len) {
    uint8_t checkSum = data[2] & 0xFF; // CRC lower byte in EEPROM line 2
    return checkSum == crcCalc(data, len);
}




void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM4){ //checks every 1 sec if the usr button is pressed. If usr btn is pressed, the program start.
		  if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin)){
			  HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, RESET);
			  picker = 1; // picker is a fundamental value (2^n for n = {0, 1,2,3,...}) that selects correct bit from a word...
			  //...for example: picker & word = 000100 & 101101 = 100.  00010 & 101101 = 0. With this logic we can sequentially select which bit to send.
			  checkmark = 1;
			  //reseting counters and setting up ProgrammingSeq array:
			  sequence_counter = 0;
			  AnswerCustomMsg_counter = 0;
			  bit_counter = 0; // bit_counter is the smallest counter, it counts from 0 to 5, to guide the FrameMatrix when sending frames.
				ProgrammingSeq[0] = 0x8400;
				ProgrammingSeq[1] = WriteWords[sequence_counter];
				ProgrammingSeq[6] = 0x8400;
				ProgrammingSeq[7] = WriteWords[sequence_counter];
				memset(AnswerCustomMsg, 0, sizeof(AnswerCustomMsg));

		  }
	}
	if(htim->Instance == TIM3){ // The main timer interrupt. Runs every 100us.


		switch(checkmark){
			case 0:
				break;
			case 1: //Powering up the sensor
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, TIFEN_Matrix[bit_counter]); //setting 0 to Aout.
				HAL_GPIO_WritePin(POWER_GPIO_Port, POWER_Pin, SET); //Turning on voltage stabilizer TC1014
				bit_counter ++ ;
				if(bit_counter >= 5){
					bit_counter = 0;
					checkmark = 2;
				}
				break;
			case 2: //Writing enter interface sequence
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0); //Triggers step-down signalizes start of a frame. (Useful only for reading with osciloscope)
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, FrameMatrix[(picker & WordABCD) && 1][bit_counter]); //FrameMatrix[x][y] has frame struction. The [x] is logic 0/1. The [y] is the sequence of the frame.
				bit_counter ++;
				if(bit_counter >= 6){
					HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,1);
					bit_counter = 0;
					picker = picker << 1; //the picker goes one bit up, 1 -> 2 -> 4 -> 8 ->... -> 32768. After that the whole word has been sent, and picker resets to picker = 1.
					if(!picker){ // When picker (uint16_t) reaches value of 65536 it will set itself to '0' since the highest number for uint16_t is 65,535. Therefore the condition !picker.
						picker = 1;
						checkmark = 3;
//						checkmark = 7; // Uncomment this line, and comment out checkmark =3; line to write custom message and read the answer from TLI4971
					}
				}
				break;
			case 3: // Reading whole EEPROM register and storing it into EPROMreg matrix (EPROMreg[0] - before change, EPROMreg[1] after change)
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0);
				if(bit_counter == 5 && ReadWords[word_counter] == 0xffff){ // 0xffff is the NOP, and during sending 0xffff we check for answer.
					EPROMreg[reg_selector][reg_counter] += picker * !HAL_GPIO_ReadPin(ANSWER_GPIO_Port, ANSWER_Pin);
				}
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, FrameMatrix[(ReadWords[word_counter] & picker) && 1][bit_counter]);
				bit_counter ++;
				if(bit_counter >= 6){
					HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,1);
					bit_counter = 0;
					picker = picker << 1;
					if(!picker){
						if(ReadWords[word_counter] == 0xffff){
						reg_counter ++;
						}
						picker = 1;
						word_counter ++;
						if(word_counter >= 38){
							reg_counter = 0;
							word_counter = 0;
							if(reg_selector){
								reg_selector = 0;
								checkmark = 0;
							}
							else{
								reg_selector = 1;
								crc_value = crcCalc(EPROMreg[0], 18);
								WriteWords[2] += crc_value;
							    checkmark = 4;// <- comment out this line for single EEPROM read
//							    checkmark = 0; //<- instert this line for single EEPROM read
							}
						}
					}
				}
				break;
			case 4:
				HAL_GPIO_WritePin(VPROG_GPIO_Port, VPROG_Pin, 0); // Set pin OCD2 to zero after 30ms signal.
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0);
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, FrameMatrix[(picker & ProgrammingSeq[word_counter]) && 1][bit_counter]);
				bit_counter ++;
				if(bit_counter >= 6){
					HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,1);
					bit_counter = 0;
					picker = picker << 1;
					if(!picker){
						picker = 1;
						word_counter ++;
						switch(word_counter){
							case 4:
							case 10:
								checkmark = 5;
								break;
							case 6:
								checkmark = 6;
								break;
							case 12:
								word_counter = 0;
								sequence_counter ++;
								if(sequence_counter >= 18){
									checkmark = 3;
								}
								else{
									ProgrammingSeq[0] += 0x10;
									ProgrammingSeq[1] = WriteWords[sequence_counter];
									ProgrammingSeq[6] += 0x10;
									ProgrammingSeq[7] = WriteWords[sequence_counter];
								}
								break;
						}
					}
				}
				break;
			case 5: // set Vprog and hold for 30ms
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0);
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, 1);
				HAL_GPIO_WritePin(VPROG_GPIO_Port, VPROG_Pin, 1);
				vprog_counter ++;
				if(vprog_counter >= 300){
					vprog_counter = 0;
					checkmark = 4;
				}
				break;
			case 6: // wait 100us
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0);
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, 1);
				checkmark = 4;
				break;
			case 7: //Sending CustomMsg and reading back the answer into AnswerCustomMsg
				if(bit_counter == 5 && AnswerSchematic[word_counter]==1){
					AnswerCustomMsg[AnswerCustomMsg_counter] += picker * !HAL_GPIO_ReadPin(ANSWER_GPIO_Port, ANSWER_Pin);
				}
				HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,0);
				HAL_GPIO_WritePin(AOUT_GPIO_Port, AOUT_Pin, FrameMatrix[(picker & CustomMsg[word_counter]) && 1][bit_counter]);
				bit_counter ++;
				if(bit_counter >= 6){
					HAL_GPIO_WritePin(TRIGGER_GPIO_Port, TRIGGER_Pin,1);
					bit_counter = 0;
					picker = picker << 1;
					if(!picker){
						picker = 1;
						if(AnswerSchematic[word_counter] == 1){
							AnswerCustomMsg_counter ++;
						}
						word_counter ++;
						if(word_counter >= custom_msg_size){
							word_counter = 0;
							checkmark = 0;
						}
					}
				}
				break;
		}
}
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
#ifdef USE_FULL_ASSERT
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
