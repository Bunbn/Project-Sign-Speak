/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 TIMER MODULE
 *******************************************************************************
 * @file           : timer.c
 * @brief          : timer module
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

#include "timer.h"

uint8_t ARR_flag = 0;

/* -----------------------------------------------------------------------------
 * function : setup_TIM2( int iDutyCycle )
 * INs      : int iDutyCycle - integer for how high the CCR will count to
 * OUTs     : none
 * action   : sets up TIM2, which includes:
 * 			  enables the clock for TIM2
 * 			  sets the ARR to 10s + rand time val
 * 			  sets the CCR to rand time val
 * 			  start the timer
 * authors  : Ethan Robson - erobson@calpoly.edu
 * version  : 0.1
 * date     : 230501
 * -------------------------------------------------------------------------- */
void setup_TIM2( int countVal ) {
   TIM2->CR1 &= ~(TIM_CR1_CEN); // turn off counter
   RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; // enable clock for TIM2
   TIM2->DIER |= (TIM_DIER_UIE); // enable event gen
   TIM2->ARR = countVal; // count to val at 16MHz
   TIM2->SR &= ~(TIM_SR_UIF); // clr IRQ flag in status reg
   NVIC->ISER[0] |= (1 << (TIM2_IRQn & 0x1F)); // set NVIC interrupt: 0x1F
   __enable_irq(); // global IRQ enable
   TIM2->CR1 |= TIM_CR1_CEN; // start TIM2 CR1
}

/* -----------------------------------------------------------------------------
 * function : TIM2_IRQHandler( )
 * INs      : none
 * OUTs     : none
 * action   : handles either a CCR or ARR event
 * 		      sets GPIOC0 to HIGH when CCR is triggered
 * 		      turns off the counter and resets GPIOC0 when ARR is triggered
 * authors  : Ethan Robson - erobson@calpoly.edu
 * version  : 0.1
 * date     : 230501
 * -------------------------------------------------------------------------- */
void TIM2_IRQHandler(void) {
   // ARR event trigger
   if (TIM2 -> SR & TIM_SR_UIF) { // triggered by ARR event
      TIM2 -> SR &= ~(TIM_SR_UIF); // manage the flag
      TIM2 -> CR1 &= ~(TIM_CR1_CEN); // turn off counter
      ARR_flag = 1; // set the ARR event flag
   }
}
