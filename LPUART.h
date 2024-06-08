/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 LPUART MODULE HEADER
 *******************************************************************************
 * @file           : LPUART.h
 * @brief          : LPUART module header
 * authors         : Ethan Robson - erobson@calpoly.edu
 * 		           : Grayson Parker - gparke09@calpoly.edu
 * 		           : Alain Kanadijan - aakandj@calpoly.edu
 * version         : 1.0
 * date            : 230424
 * compiler        : STM32CubeIDE v.1.15.0
 * target          : NUCLEO-L496ZG
 * clocks          : 4 MHz MSI to AHB2
 * @attention      : (c) 2023 STMicroelectronics.  All rights reserved.
 *******************************************************************************
 * REVISION HISTORY
 * 1.0 240422 Finished making LCD module
 *******************************************************************************
 * 45678-1-2345678-2-2345678-3-2345678-4-2345678-5-2345678-6-2345678-7-234567 */
/* USER CODE END Header */

#ifndef INC_LPUART_H_
#define INC_LPUART_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32l496xx.h" // include all custom GPIO name declarations

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Function prototypes
void LPUART_Init(void);
void LPUART_Print(const char * message);
void LPUART1_IRQHandler(void);
void LPUART_ESC_Print(const char * message);
void printColumn(int row, int column, const char text[]);
void terminal(void);
void updateTerminal(char Letter);
void handReact(uint8_t fingers[5]);
void bottomHand(void);
char createWord(uint8_t fingers[5]);


#endif /* INC_LPUART_H_ */
