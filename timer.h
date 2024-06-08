/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 TIMER MODULE HEADER
 *******************************************************************************
 * @file           : timer.h
 * @brief          : timer module header
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

#ifndef INC_TIMER_H_
#define INC_TIMER_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h" // include all custom GPIO name declarations

/* Function Prototypes -------------------------------------------------------*/
void setup_TIM2( int countVal );
void TIM2_IRQHandler(void);

/* Variable Declarations -----------------------------------------------------*/
extern uint8_t ARR_flag;

#endif /* INC_TIMER_H_ */
