/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 I2C RECEIVER MODULE
 *******************************************************************************
 * @file           : I2C_recv.c
 * @brief          : I2C receiver module
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

#include "I2C_recv.h"

/* Variable Declarations -----------------------------------------------------*/
uint32_t I2C_data = 0; // global variable for I2C received data
uint8_t I2C_ready = 0; // global flag for if I2C receive is complete

/* -----------------------------------------------------------------------------
 * function : I2C_Init( )
 * INs      : none
 * OUTs     : none
 * action   : Inits I2C1 on board as a peripheral
 *            Sets address as 0x11
 *            Sets up interrupts on address receive, data receive, and stops
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void I2C_Init(void) {
   // configure PB8 as I2C1_SCL and PB9 as I2C1_SDA
   RCC -> AHB2ENR |= (RCC_AHB2ENR_GPIOBEN); // GPIOB enable

   // configs the registers in order: OTYPER, PUPDR, OSPEEDR, MODDER
   GPIOB -> OTYPER &= ~(GPIO_OTYPER_OT8 | // PB8: SCL
		   	   	   	    GPIO_OTYPER_OT9); // PB9: SDA
   GPIOB -> OTYPER |= (GPIO_OTYPER_OT8 |
		   	   	   	   GPIO_OTYPER_OT9); // set both PB8 and PB9 to open drain
   GPIOB -> PUPDR &= ~(GPIO_PUPDR_PUPD8 |
		   	   	   	   GPIO_PUPDR_PUPD9); // use ext. pullups
   GPIOB -> OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED8_Pos) |
		   	   	   	    (3 << GPIO_OSPEEDR_OSPEED9_Pos)); // maximum speed
   GPIOB -> MODER &= ~(GPIO_MODER_MODE8 |
		   	   	   	   GPIO_MODER_MODE9); // reset the GPIO ports
   GPIOB -> MODER |= (GPIO_MODER_MODE8_1 |
		   	   	   	  GPIO_MODER_MODE9_1); // alternate function mode

   // set the alternate function modes for PB8 and PB9
   // use AFRH, the upper part of AFR register
   GPIOB -> AFR[1] &= ~((0x000F << GPIO_AFRH_AFSEL8_Pos)); // PB8
   GPIOB -> AFR[1] |=  ((0x0004 << GPIO_AFRH_AFSEL8_Pos)); // AF4

   GPIOB -> AFR[1] &= ~((0x000F << GPIO_AFRH_AFSEL9_Pos)); // PB9
   GPIOB -> AFR[1] |=  ((0x0004 << GPIO_AFRH_AFSEL9_Pos)); // AF4

   // Configure I2C1
   RCC -> APB1ENR1 |= RCC_APB1ENR1_I2C1EN; // enable I2C bus clock
   I2C1 -> CR1 &= ~(I2C_CR1_PE); // put I2C into reset (release SDA, SCL)
   I2C1 -> CR1 |= (I2C_CR1_ADDRIE); // address match interrupt enable
   I2C1 -> CR1 |= (I2C_CR1_RXIE); // receive interrupt enable
   I2C1 -> CR1 |= (I2C_CR1_STOPIE); // receive interrupt enable
   I2C1 -> TIMINGR = 0x00303E8D; // 76kHz I2C clock

   // Configure own address for I2C1
   I2C1 -> OAR1 &= ~(I2C_OAR1_OA1EN); // clear own address 1
   I2C1 -> OAR2 &= ~(I2C_OAR2_OA2EN); // clear own address 2
   I2C1 -> OAR1 &= ~(I2C_OAR1_OA1MODE); // 7 bit address
   I2C1 -> OAR1 |= (34) << I2C_OAR1_OA1_Pos; // set address to 0x11 (0x11<<1)
   I2C1 -> OAR1 |= (I2C_OAR1_OA1EN); // enable own address 1

   NVIC -> ISER[0] |= (1 << (I2C1_EV_IRQn & 0x1F)); // enable I2C1 event ISR
   __enable_irq(); // enable global interrupts

   I2C1 -> CR1 |= (I2C_CR1_PE); // enable I2C
}

/* -----------------------------------------------------------------------------
 * function : I2C_Init( )
 * INs      : none
 * OUTs     : none
 * action   : Interrupt handler for I2C events
 *            On address match event flag clear I2C ready flag and data reg
 *            On data ready flag shift next byte into data reg
 *            On stop condition flag set I2C ready flag
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void I2C1_EV_IRQHandler(void) {
    if (I2C1->ISR & I2C_ISR_ADDR) { // Address match event flag
        I2C1->ICR |= I2C_ICR_ADDRCF; // Clear address flag
        I2C_data = 0; // reset data value
        I2C_ready = 0;
    }

    // Check if receive register is ready
    if (I2C1->ISR & I2C_ISR_RXNE) {
    	I2C_data = (I2C_data << 8) | I2C1->RXDR; // Read received data
    }

    // Check for stop condition and when data reg is empty
    if ((I2C1->ISR & I2C_ISR_STOPF) && !(I2C1->ISR & I2C_ISR_RXNE)) {
        I2C1->ICR |= I2C_ICR_STOPCF; // Clear stop flag
        I2C_ready = 1; // data is now ready
    }
}
