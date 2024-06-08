/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 FINAL PROJECT - SIGN SPEAK
 *******************************************************************************
 * @file           : main.c
 * @brief          : SIGN SPEAK NUCLEO MODULE
 * authors         : Ethan Robson - erobson@calpoly.edu
 * 		    	   : Grayson Parker - gparke09@calpoly.edu
 * 		           : Alain Kanadijan - aakandj@calpoly.edu
 * version         : 1.0
 * date            : 240607
 * compiler        : STM32CubeIDE v.1.15.0
 * target          : NUCLEO-L496ZG
 * clocks          : 16 MHz MSI to AHB2
 * @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
 *******************************************************************************
 * REVISION HISTORY
 * 1.0 240607 Finished making code
 *******************************************************************************
 * 45678-1-2345678-2-2345678-3-2345678-4-2345678-5-2345678-6-2345678-7-234567 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "LPUART.h"
#include "I2C_recv.h"
#include "timer.h"
#include <stdint.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);
uint8_t compareArrays(uint8_t arr1[], uint8_t arr2[], uint8_t size);
uint8_t convert_Finger_Data(uint16_t data, uint8_t fingerNum);

/* -----------------------------------------------------------------------------
 * function : main( )
 * INs      : none
 * OUTs     : none
 * action   : Inits I2C1
 * 			  Init LPUART
 * 			  Clear terminal screen, hide cursor
 * 			  Print terminal
 * 			  Print bottom hand
 * 			  Init TIM2
 * 			  Init FingerNum, Data, and Prev finger data vars
 * 			  In loop:
 * 			  If I2C data ready:
 * 			  Check if prev data == new data
 * 			  If datas dont match:
 * 			  Reset timer count
 * 			  Manage I2C data ready flag
 * 			  If ARR flag:
 * 			  Update terminal with char
 * 			  Manage ARR ready flag
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
int main(void) {
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_I2C1_Init();

	I2C_Init(); // init I2C1

	LPUART_Init(); // init LPUART
	LPUART_ESC_Print("[?25l"); // hide cursor
	LPUART_ESC_Print("[2J"); // clear terminal

	terminal(); // print terminal

	bottomHand(); // print bottom part of hand

	setup_TIM2(32000000); // count to 2 seconds at 16 MHz clock rate

	// define vars to use in the loop
	uint8_t fingerNum = 0;
	uint8_t fingerData[5] = { 0, 0, 0, 0, 0 };
	uint8_t prevFingerData[5] = { 0, 0, 0, 0, 0 };

	while (1) {
		// check if I2C data is ready
		if (I2C_ready) {

			I2C_ready = 0; // clear flag until next batch of data arrives

			// update prev finger data value
			for (int i = 0; i < 5; i++) {
				prevFingerData[i] = fingerData[i];
			}

			// mask upper 3 bits and shift to first bit positions
			fingerNum = I2C_data >> 13;

			// obtain new finger data
			// mask bits 3 through 12 (count starts at 0)
			fingerData[fingerNum] = convert_Finger_Data(
					(I2C_data & 0x1FF8) >> 3, fingerNum);

			// check to see if finger input has changed
			if (!compareArrays(prevFingerData, fingerData, 5)) {
				// reset timer and start the count again
				TIM2->CR1 &= ~(TIM_CR1_CEN); // turn off counter
				TIM2->CNT = 0x00000000; // reset the count

				handReact(fingerData); // update fingers in terminal

				TIM2->CR1 |= TIM_CR1_CEN; // start the counter

			}

			I2C_ready = 0; // clear flag until next batch of data arrives
		}

		// check if ARR count has overflowed
		if (ARR_flag) {
			// convert ADC data to letter
			char letter = createWord(fingerData);
			updateTerminal(letter); // print letter to terminal
			ARR_flag = 0; // manage ARR flag
		}
	}
}

/* -----------------------------------------------------------------------------
 * function : compareArrays( )
 * INs      : uint8_t arr1[] - array 1
 *            uint8_t arr2[] - array 2
 *            uint8_t size - size of arrays
 * OUTs     : none
 * action   : Loop through each val in array and see if they are equal
 *            If not equal, return a 0
 *            If all elements in array are equal, return a 1
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * 		    : Chat GPT
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
uint8_t compareArrays(uint8_t arr1[], uint8_t arr2[], uint8_t size) {
	for (int i = 0; i < size; i++) {
		if (arr1[i] != arr2[i]) {
			return 0; // If any element is different, arrays are not equal
		}
	}
	return 1; // All elements are equal
}

/* -----------------------------------------------------------------------------
 * function : convert_Finger_Data( )
 * INs      : uint16_t data - ADC data
 *            uint8_t fingerNum - finger number
 * OUTs     : uint8_t - 0, 1, 2 for finger value
 *            2 - Finger fully open
 *            1 - Finger half closed
 *            0 - Finger fully closed
 * action   : Adjust finger data for each finger num based on calibration
 *            Return values for finger data based on thresholds
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
uint8_t convert_Finger_Data(uint16_t data, uint8_t fingerNum) {
	// adjust finger data for fingers based on manual calibration
	if (fingerNum == 0) {
		data -= 30;
	}
	else if (fingerNum == 1) {
		data -= 0;
	}
	else if (fingerNum == 2) {
		data -= 0;
	}
	else if (fingerNum == 3) {
		data += 0;
	}

	// store a 0, 1, or 2 based on thresholds
	// fingerNum 4 has special thresholds since the resistance was very high
	if (data < 360 && fingerNum != 4) {
		return 2;
	}
	else if (data < 440 && fingerNum != 4) {
		return 1;
	}
	else if (data < 500 && fingerNum == 4) {
		return 2;
	}
	else if (data < 550 && fingerNum == 4) {
		return 1;
	}
	else { // fully closed for both statements
		return 0;
	}
}

/* -----------------------------------------------------------------------------
 * function : MX_I2C1_Init ( void )
 * INs      : none
 * OUTs     : none
 * action   : configures I2C1
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
static void MX_I2C1_Init(void) {
	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x00303D5B;
	hi2c1.Init.OwnAddress1 = 34;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1)
			!= HAL_OK) {
		Error_Handler();
	}

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_8;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
