/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 I2C RECEIVER MODULE HEADER
 *******************************************************************************
 * @file           : I2C_recv.h
 * @brief          : I2C receiver module header
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

#ifndef INC_I2C_RECV_H_
#define INC_I2C_RECV_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h" // include all custom GPIO name declarations
#include "main.h"
#include <stdint.h>
#include <stdio.h>

/* Function Prototypes -------------------------------------------------------*/
void I2C_Init(void);
void I2C1_EV_IRQHandler(void);

/* Variable Declarations -----------------------------------------------------*/
extern uint32_t I2C_data; // global variable for I2C received data
extern uint8_t I2C_ready; // global flag for if I2C receive is complete

#endif /* INC_I2C_RECV_H_ */
