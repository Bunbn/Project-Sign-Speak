/* USER CODE BEGIN Header */
/*******************************************************************************
 * EE 329 LPUART MODULE
 *******************************************************************************
 * @file           : LPUART.c
 * @brief          : LPUART module
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

#include "LPUART.h"

/* -----------------------------------------------------------------------------
 * function : LPUART_init( )
 * INs      : none
 * OUTs     : none
 * action   : Initializes the LPUART1 port on GPIOG
 * 			  Sets up interrupt for incoming UART data
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240508
 * -------------------------------------------------------------------------- */
void LPUART_Init(void) {
	PWR->CR2 |= (PWR_CR2_IOSV); // power avail on PG[15:2] (LPUART1)
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOGEN); // enable GPIOG clock
	RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN; // enable LPUART clock bridge

	// configs the registers in order: AFR, OTYPER, PUPDR, OSPEEDR, MODDER
	// configure the AFRL for PG7
	GPIOG->AFR[0] &= ~(GPIO_AFRL_AFSEL7); // reset the bits
	GPIOG->AFR[0] |= GPIO_AFRL_AFSEL7_3; // set PG7 to AF8 for LPUART1
	// configure the AFRH for PG8
	GPIOG->AFR[1] &= ~(GPIO_AFRH_AFSEL8); // reset the bits
	GPIOG->AFR[1] |= GPIO_AFRH_AFSEL8_3; // set PG7 to AF8 for LPUART1

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);
	GPIOG->OTYPER &= ~(GPIO_OTYPER_OT7 | // PG7: TX
			GPIO_OTYPER_OT8); // PG8: RX
	GPIOG->PUPDR &= ~(GPIO_PUPDR_PUPD7 |
	GPIO_PUPDR_PUPD8); // no PUPDRs for both
	GPIOG->OSPEEDR |= ((3 << GPIO_OSPEEDR_OSPEED7_Pos)
			| (3 << GPIO_OSPEEDR_OSPEED8_Pos)); // maximum speed
	GPIOG->MODER &= ~(GPIO_MODER_MODE7 |
	GPIO_MODER_MODE8); // reset the GPIO ports
	GPIOG->MODER |= (GPIO_MODER_MODE7_1 |
	GPIO_MODER_MODE8_1); // alternate function mode

	// select UART pins, P/P, no PU/PD, high speed, alt. fcn mode
	// enable UART clock, set data size
	LPUART1->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0); // 8-bit data
	LPUART1->CR1 |= USART_CR1_UE; // enable LPUART1
	LPUART1->CR1 |= (USART_CR1_TE | USART_CR1_RE); // enable xmit & recv
	LPUART1->CR1 |= USART_CR1_RXNEIE; // enable LPUART1 recv interrupt
	LPUART1->ISR &= ~(USART_ISR_RXNE); // clear Recv-Not-Empty flag

	// LPUART clk speed is 16MHz, goal to get 115200 baud
	// 256*LPUART_CLK/BRR = baud
	// BRR calculates to 35555
	LPUART1->BRR = 35555; // set LPUART_DIV value

	// enable UART, Tx/Rx, interrupts
	NVIC->ISER[2] |= (1 << (LPUART1_IRQn & 0x1F)); // enable LPUART1 ISR
	__enable_irq(); // enable global interrupts
}

/* -----------------------------------------------------------------------------
 * function : LPUART_Print( const char *message )
 * INs      : pointer to char message
 * OUTs     : none
 * action   : Writes the character of each message
 * 			  If a string, loops through each char and writes it to serial
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240508
 * -------------------------------------------------------------------------- */
void LPUART_Print(const char *message) {
	uint16_t iStrIdx = 0;
	while (message[iStrIdx] != 0) {
		while (!(LPUART1->ISR & USART_ISR_TXE))
			// wait for empty xmit buffer
			;
		LPUART1->TDR = message[iStrIdx]; // send this character
		iStrIdx++; // advance index to next char
	}
}

/* -----------------------------------------------------------------------------
 * function : LPUART_ESC_Print( const char *message )
 * INs      : pointer to char MESSAGE, ignore the ESC in input
 * OUTs     : none
 * action   : Specifically handles the ESC functions
 * 			  Form of ESC: ESCMESSAGE
 * 			  Input is assumed to just be MESSAGE, ESC input is implied
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240508
 * -------------------------------------------------------------------------- */
void LPUART_ESC_Print(const char *message) {
	while (!(LPUART1->ISR & USART_ISR_TXE)) { // wait for empty xmit buffer
		;
	}
	LPUART1->TDR = 0x1B; // send the ESC code
	LPUART_Print(message); // print the rest of the ESC sequence
}

/* -----------------------------------------------------------------------------
 * function : LPUART1_IRQHandler( void )
 * INs      : none
 * OUTs     : none
 * action   : Receives interrupt whenever a keyboard key is pressed
 * 			  Checks if any special characters are pressed
 * 			  If RGBW is pressed, changes text color
 * 			  If arrow keys are pressed, updates player position
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240508
 * -------------------------------------------------------------------------- */
void LPUART1_IRQHandler(void) {
	uint8_t charRecv;
	if (LPUART1->ISR & USART_ISR_RXNE) {
		charRecv = LPUART1->RDR;
		switch (charRecv) {
		case 0x1B: // if one of the arrows are pressed
			charRecv = LPUART1->RDR; // receive [
			break;
		case 'R':
			// when R is pressed, changes character color to red
			LPUART_ESC_Print("[31m");
			break;
		case 'G':
			// when G is pressed, changes character color to green
			LPUART_ESC_Print("[32m");
			break;
		case 'B':
			// when B is pressed, changes character color to blue
			LPUART_ESC_Print("[34m");
			break;
		case 'W':
			// when W is pressed, changes character color to white
			LPUART_ESC_Print("[37m");
			break;
		default:
			while (!(LPUART1->ISR & USART_ISR_TXE))
				; // wait for empty TX buffer
			LPUART1->TDR = charRecv; // echo char to terminal
		} // end switch
	}
}

/* -----------------------------------------------------------------------------
 * function : printColumn( )
 * INs      : int row - row number
 *            int column - column number
 *            const char text[] - string of what to print
 * OUTs     : none
 * action   : Concatenates string to limit of terminal boundaries
 *            Prints out string at designated location
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240508
 * -------------------------------------------------------------------------- */
void printColumn(int row, int column, const char text[]) {
	char escape[3000];
	if ((90 > row) | (row > 0)) {
		if ((90 > column) | (column > 0)) {
			snprintf(escape, sizeof(escape), "\x1B[%d;%dH", row, column);
			LPUART_ESC_Print("[H");
			while (!(LPUART1->ISR & USART_ISR_TXE)) { // wait for empty xmit buffer
				;
			}
			strcat(escape, text);
			//sprintf(escape, "\x1B[%d;%dH", row, column);
			LPUART_ESC_Print(escape);

		}
	}

}

/* -----------------------------------------------------------------------------
 * function : updateTerminal( )
 * INs      : char Letter - letter to update the ASL terminal with
 * OUTs     : none
 * action   : Convert input character to a string
 *            Print letter at fixed position
 *            Update counter and location
 *            Check for row and column overflows
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void updateTerminal(char Letter) {
	// Static variables to maintain state between function calls
	static int counter = 0;    // Letter counter
	static int location = 5;   // Current column position
	static int row = 5;        // Current row position

	// Convert the input character to a string for printing purposes
	char strLetter[2] = { Letter, '\0' };

	// Print the letter at the current row and location
	printColumn(row, location, strLetter);

	// Also print the letter at a fixed position (row 3, column 32)
	printColumn(3, 32, strLetter);

	// Update the location and counter
	location++;
	counter++;

	// Check if 30 letters have been printed in the current row
	if (counter >= 30) {
		// Move to the next row
		row++;

		// Reset location and counter for the new row
		location = 5;
		counter = 0;
	}

	// If the row exceeds 9, reset to the initial state
	if (row > 9) {
		row = 5;
		location = 5;
		counter = 0;

		// Clear the terminal rows from 5 to 9 between columns 5 and 34
		for (int i = 5; i < 35; i++) {
			printColumn(5, i, " ");
		}
		for (int i = 5; i < 35; i++) {
			printColumn(6, i, " ");
		}
		for (int i = 5; i < 35; i++) {
			printColumn(7, i, " ");
		}
		for (int i = 5; i < 35; i++) {
			printColumn(8, i, " ");
		}
		for (int i = 5; i < 35; i++) {
			printColumn(9, i, " ");
		}
	}
}

/* -----------------------------------------------------------------------------
 * function : terminal( )
 * INs      : pointer to char message
 * OUTs     : none
 * action   : Prints ASL terminal to the serial terminal
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void terminal(void) {
	printColumn(1, 0, "  ASL to English Translator  ");
	printColumn(2, 0, "------------------------------------------");
	printColumn(3, 0, "|			             	 |");
	printColumn(4, 0, "|			            	 |");
	printColumn(5, 0, "|			            	 |");
	printColumn(6, 0, "|			            	 |");
	printColumn(7, 0, "|			            	 |");
	printColumn(8, 0, "|			            	 |");
	printColumn(9, 0, "|			            	 |");
	printColumn(10, 0, "------------------------------------------");
	printColumn(3, 0, "| Your translated letter is: ");
	printColumn(4, 0, "| So far you have:");
}

/* -----------------------------------------------------------------------------
 * function : handReact( )
 * INs      : uint8_t fingers[5] - array of finger data
 * OUTs     : none
 * action   : Prints the fingers depending on the finger data
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void handReact(uint8_t fingers[5]) {
	// pinky finger
	switch (fingers[0]) {
	case 0:
		printColumn(14, 0, "       ");
		printColumn(15, 0, "        ");
		printColumn(16, 0, "         ");
		printColumn(17, 0, "         ");
		printColumn(18, 0, "          ");
		printColumn(19, 0, "           ");
		printColumn(20, 0, "           ");
		printColumn(21, 0, "            ");
		printColumn(22, 0, "             ");
		printColumn(23, 0, "              ");
		printColumn(24, 0, "               ");
		printColumn(25, 0, "                ");
		printColumn(26, 0, "                ");
		printColumn(27, 0, "                ");
		printColumn(28, 0, "      .--==-----:");
		break;
	case 1:
		printColumn(14, 0, "       ");
		printColumn(15, 0, "        ");
		printColumn(16, 0, "         ");
		printColumn(17, 0, "         ");
		printColumn(18, 0, "          ");
		printColumn(19, 0, "           ");
		printColumn(20, 0, "           ");
		printColumn(21, 0, "            ");
		printColumn(22, 0, "             ");
		printColumn(23, 0, "      ---==++=");
		printColumn(24, 0, "     .-==----=-");
		printColumn(25, 0, "     .---:-----.");
		printColumn(26, 0, "      ::-------=");
		printColumn(27, 0, "      -----===-:");
		printColumn(28, 0, "      .--==-----:");
		break;
	case 2:
		printColumn(14, 0, "  :--=:");
		printColumn(15, 0, ".:----=-");
		printColumn(16, 0, ".------=.");
		printColumn(17, 0, " :-------");
		printColumn(18, 0, " :::------");
		printColumn(19, 0, "  :---===+.");
		printColumn(20, 0, "  :-=======");
		printColumn(21, 0, "   :--------");
		printColumn(22, 0, "   .:------=-");
		printColumn(23, 0, "    :----==++=");
		printColumn(24, 0, "     --==----=-");
		printColumn(25, 0, "     .---:-----.");
		printColumn(26, 0, "      ::-------=");
		printColumn(27, 0, "      -----===-:");
		printColumn(28, 0, "      .--==-----:");
		break;
	default:
		break;
	}

	// ring finger
	switch (fingers[1]) {
	case 0:
		printColumn(11, 18, "         ");
		printColumn(12, 17, "          ");
		printColumn(13, 17, "          ");
		printColumn(14, 16, "           ");
		printColumn(15, 16, "          ");
		printColumn(16, 16, "          ");
		printColumn(17, 16, "          ");
		printColumn(18, 16, "          ");
		printColumn(19, 16, "           ");
		printColumn(20, 16, "          ");
		printColumn(21, 16, "           ");
		printColumn(22, 16, "           ");
		printColumn(23, 16, "            ");
		printColumn(24, 16, "            ");
		printColumn(25, 16, "            ");
		printColumn(26, 16, "            ");
		printColumn(27, 16, "            ");
		printColumn(28, 16, "  -=========-");
		break;
	case 1:
		printColumn(11, 18, "         ");
		printColumn(12, 17, "          ");
		printColumn(13, 17, "          ");
		printColumn(14, 16, "           ");
		printColumn(15, 16, "          ");
		printColumn(16, 16, "          ");
		printColumn(17, 16, "          ");
		printColumn(18, 16, "          ");
		printColumn(19, 16, "           ");
		printColumn(20, 16, "          ");
		printColumn(21, 16, "           ");
		printColumn(22, 16, "  .-=====- ");
		printColumn(23, 16, " ---:-----=.");
		printColumn(24, 16, " ---------=:");
		printColumn(25, 16, " ----------.");
		printColumn(26, 16, "  ---------:");
		printColumn(27, 16, "  --------=-");
		printColumn(28, 16, "  -=========-");
		break;
	case 2:
		printColumn(11, 18, ".-=====- ");
		printColumn(12, 17, ".----====.");
		printColumn(13, 17, ".---=====:");
		printColumn(14, 16, "  ---=====:");
		printColumn(15, 16, " :----====");
		printColumn(16, 16, " :--======");
		printColumn(17, 16, " :--======");
		printColumn(18, 16, " :========");
		printColumn(19, 16, " .:------=.");
		printColumn(20, 16, " -------==");
		printColumn(21, 16, " -------==.");
		printColumn(22, 16, " .========+");
		printColumn(23, 16, " ---:-----=.");
		printColumn(24, 16, " ---------=:");
		printColumn(25, 16, " ----------.");
		printColumn(26, 16, "  ---------:");
		printColumn(27, 16, "  --------=-");
		printColumn(28, 16, "  -=========-");
		break;
	default:
		break;
	}

	// middle finger
	switch (fingers[2]) {
	case 0:
		printColumn(11, 32, "           ");
		printColumn(12, 32, "           ");
		printColumn(13, 32, "           ");
		printColumn(14, 32, "           ");
		printColumn(15, 32, "           ");
		printColumn(16, 32, "           ");
		printColumn(17, 32, "           ");
		printColumn(18, 32, "           ");
		printColumn(19, 32, "           ");
		printColumn(20, 32, "           ");
		printColumn(21, 32, "           ");
		printColumn(22, 32, "           ");
		printColumn(23, 32, "           ");
		printColumn(24, 32, "           ");
		printColumn(25, 32, "           ");
		printColumn(26, 32, "           ");
		printColumn(27, 32, "           ");
		printColumn(28, 32, "===---==++.");
		break;
	case 1:
		printColumn(11, 32, "           ");
		printColumn(12, 32, "           ");
		printColumn(13, 32, "           ");
		printColumn(14, 32, "           ");
		printColumn(15, 32, "           ");
		printColumn(16, 32, "           ");
		printColumn(17, 32, "           ");
		printColumn(18, 32, "           ");
		printColumn(19, 32, "           ");
		printColumn(20, 32, "           ");
		printColumn(21, 32, " =-------= ");
		printColumn(22, 32, "=---------:");
		printColumn(23, 32, "=---------:");
		printColumn(24, 32, "=---------.");
		printColumn(25, 32, "==-------- ");
		printColumn(26, 32, "-=-------- ");
		printColumn(27, 32, "-====---== ");
		printColumn(28, 32, "===---==++.");
		break;
	case 2:
		printColumn(11, 32, "  :--==-.  ");
		printColumn(12, 32, " .-------: ");
		printColumn(13, 32, " :-----=-- ");
		printColumn(14, 32, " :-----==-.");
		printColumn(15, 32, " :----=---.");
		printColumn(16, 32, " :--------.");
		printColumn(17, 32, " --======- ");
		printColumn(18, 32, ".---------.");
		printColumn(19, 32, ":--=-==---:");
		printColumn(20, 32, "===-======-");
		printColumn(21, 32, "++==--===+-");
		printColumn(22, 32, "=---------:");
		printColumn(23, 32, "=---------:");
		printColumn(24, 32, "=---------.");
		printColumn(25, 32, "==-------- ");
		printColumn(26, 32, "-=-------- ");
		printColumn(27, 32, "-====---== ");
		printColumn(28, 32, "===---==++.");
		break;
	default:
		break;
	}

	// index finger
	switch (fingers[3]) {
	case 0:
		printColumn(11, 51, "      ");
		printColumn(12, 51, "       ");
		printColumn(13, 50, "         ");
		printColumn(14, 50, "         ");
		printColumn(15, 49, "          ");
		printColumn(16, 49, "          ");
		printColumn(17, 49, "          ");
		printColumn(18, 49, "          ");
		printColumn(19, 48, "          ");
		printColumn(20, 48, "          ");
		printColumn(21, 43, "               ");
		printColumn(22, 43, "              ");
		printColumn(23, 43, "              ");
		printColumn(24, 43, "              ");
		printColumn(25, 43, "             ");
		printColumn(26, 43, "             ");
		printColumn(27, 43, "            ");
		printColumn(28, 43, "-===------=-");
		break;
	case 1:
		printColumn(11, 51, "      ");
		printColumn(12, 51, "       ");
		printColumn(13, 50, "         ");
		printColumn(14, 50, "         ");
		printColumn(15, 49, "          ");
		printColumn(16, 49, "          ");
		printColumn(17, 49, "          ");
		printColumn(18, 49, "          ");
		printColumn(19, 48, "          ");
		printColumn(20, 48, "          ");
		printColumn(21, 43, "    -========-  ");
		printColumn(22, 43, "   .+=======--");
		printColumn(23, 43, "   ==========-");
		printColumn(24, 43, "   ===------=.");
		printColumn(25, 43, "  :===-------");
		printColumn(26, 43, " .====-----=:");
		printColumn(27, 43, " -==------=-");
		printColumn(28, 43, "-===------=-");
		break;
	case 2:
		printColumn(11, 51, " :==-:");
		printColumn(12, 51, "====---");
		printColumn(13, 50, ":=------:");
		printColumn(14, 50, "==------:");
		printColumn(15, 49, ".==------:");
		printColumn(16, 49, ":=-------:");
		printColumn(17, 49, ":==--==--:");
		printColumn(18, 49, "=+=====--.");
		printColumn(19, 48, ":=--------");
		printColumn(20, 48, "==-------:");
		printColumn(21, 43, "    ===-===-=-.");
		printColumn(22, 43, "   .+=======--");
		printColumn(23, 43, "   ==========-");
		printColumn(24, 43, "   ===------=.");
		printColumn(25, 43, "  :===-------");
		printColumn(26, 43, " .====-----=:");
		printColumn(27, 43, " -==------=-");
		printColumn(28, 43, "-===------=-");
		break;
	default:
		break;
	}

	// thumb
	switch (fingers[4]) {
	case 0:
		printColumn(29, 61, "                           ");
		printColumn(30, 61, "                            ");
		printColumn(31, 61, "                           ");
		printColumn(32, 61, "                        ");
		printColumn(33, 61, "                     ");
		printColumn(34, 61, "                     ");
		printColumn(35, 61, "                    ");
		printColumn(36, 61, "                  ");
		printColumn(37, 61, "                   ");
		printColumn(38, 61, ".======+=++***+ ");
		break;
	case 1:
		printColumn(29, 61, "                           ");
		printColumn(30, 61, "                            ");
		printColumn(31, 61, "                           ");
		printColumn(32, 61, "                        ");
		printColumn(33, 61, "                     ");
		printColumn(34, 61, "     -===========+.   ");
		printColumn(35, 61, "    :==========++++.");
		printColumn(36, 61, "   -=-======++++*=");
		printColumn(37, 61, " .==-====++++***.  ");
		printColumn(38, 61, ".======+=++***+ ");
		break;
	case 2:
		printColumn(29, 61, "               :-========-.");
		printColumn(30, 61, "            -=============-:");
		printColumn(31, 61, "         .================-");
		printColumn(32, 61, "        -===========++=.");
		printColumn(33, 61, "      -=======-===+++");
		printColumn(34, 61, "      -=======-===+++");
		printColumn(35, 61, "    :==========++++.");
		printColumn(36, 61, "   -=-======++++*=");
		printColumn(37, 61, " .==-====++++***.  ");
		printColumn(38, 61, ".======+=++***+ ");
		break;
	default:
		break;
	}

}

/* -----------------------------------------------------------------------------
 * function : bottomHand( )
 * INs      : none
 * OUTs     : none
 * action   : Prints the bottom of the hand to the serial terminal
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
void bottomHand(void) {

	printColumn(29, 7, ":----===----------------------==-----------------");
	printColumn(30, 7, ":----===----------------------==-----------------");
	printColumn(31, 7, ":--=-------------::---------==--------------=-==-.");
	printColumn(32, 7, ":-::::-------------------===----------====-==+===.");
	printColumn(33, 7, ".::::::--------------====---------=====--=++==-=+:");
	printColumn(34, 7, ".::::-------------====---------==+=---===========+");
	printColumn(35, 7, ".:::-----------===----------====----===-==========.");
	printColumn(36, 7, " ::::--------==-----------==-------==-----====++++-");
	printColumn(37, 7, " :::---===--------------==--------==-------======++=");
	printColumn(38, 7, " :-------------------===---=-----==---=-----====++*+-");
	printColumn(39, 7,
		".::::::-----------==-----==----==--=-----=-====+**+=+====++=++***.");
	printColumn(40, 7,
		".::::::---------==-------=----===----==-======+++=+======+++++*+ ");
	printColumn(41, 7,
		" -::::-----------------===---=======--=======--====++==+++++**+ ");
	printColumn(42, 7,
		" --::::--------------========+=---=----=-==========+++++++++*+.");
	printColumn(43, 7,
		" :--::::-------------=======+======================++==+++++++");
	printColumn(44, 7,
		"  ----::----------=========+========================++=+++**+.");
	printColumn(45, 7,
		"  -=-:::----------===================================*++****=");
	printColumn(46, 7,
		"   =---:----------========+==========-==========+=++++*****:");
	printColumn(47, 7,
		"   -=-------------======-===-==========-========++++*****:");
	printColumn(48, 7,
		"    ==---------------=--==---=----===-======++++++****+:");
	printColumn(49, 7,
		"    .=------------------==--=---==--======++++++*****-");
	printColumn(50, 7, "     :---------------=====---===========++++++*****+");
	printColumn(51, 7, "      ---------=======+=+========++++++++++++*****:");
	printColumn(52, 7, "      .-----=========+=++++++++++++*++++*+******+");
	printColumn(53, 7, "       .==-=========+==+++***++****************.");
	printColumn(54, 7, "       .=====++++++++++******#***#*********=.");

}

/* -----------------------------------------------------------------------------
 * function : createWord( )
 * INs      : uint8_t fingers[5] - array of finger data
 * OUTs     : none
 * action   : Returns a character depending on finger data given:
 *            0 Closed
 *            1 Halfway
 *            2 Up
 *            fingers[0]=pinky
 *            fingers[1]=ring
 *            fingers[2]=middle
 *            fingers[3]=pointer
 *            fingers[4]=thumb
 * authors  : Ethan Robson - erobson@calpoly.edu
 * 		    : Grayson Parker - gparke09@calpoly.edu
 * 		    : Alain Kanadijan - aakandj@calpoly.edu
 * version  : 0.1
 * date     : 240607
 * -------------------------------------------------------------------------- */
char createWord(uint8_t fingers[5]) {
	char letter = ' '; // empty space if garbage is read

	// if statements to return a character depending on finger data
	if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 0 && fingers[4] == 2) {
		letter = 'A';
	} else if (fingers[0] == 2 && fingers[1] == 2 && fingers[2] == 2
			&& fingers[3] == 2 && fingers[4] == 0) {
		letter = 'B';
	} else if (fingers[0] == 1 && fingers[1] == 1 && fingers[2] == 1
			&& fingers[3] == 1 && fingers[4] == 2) {
		letter = 'C';
	} else if (fingers[0] == 1 && fingers[1] == 1 && fingers[2] == 1
			&& fingers[3] == 2 && fingers[4] == 1) {
		letter = 'D';
	} else if (fingers[0] == 1 && fingers[1] == 1 && fingers[2] == 1
			&& fingers[3] == 1 && fingers[4] == 0) {
		letter = 'E';
	} else if (fingers[0] == 2 && fingers[1] == 2 && fingers[2] == 2
			&& fingers[3] == 1 && fingers[4] == 1) {
		letter = 'F';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 2 && fingers[4] == 1) {
		letter = 'G';
	} else if (fingers[0] == 2 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 0 && fingers[4] == 1) {
		letter = 'I';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 2
			&& fingers[3] == 2 && fingers[4] == 2) {
		letter = 'K';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 2 && fingers[4] == 2) {
		letter = 'L';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 0 && fingers[4] == 1) {
		letter = 'M';
	} else if (fingers[0] == 1 && fingers[1] == 1 && fingers[2] == 1
			&& fingers[3] == 1 && fingers[4] == 1) {
		letter = 'O';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 2
			&& fingers[3] == 2 && fingers[4] == 1) {
		letter = 'U';
	} else if (fingers[0] == 0 && fingers[1] == 2 && fingers[2] == 2
			&& fingers[3] == 2 && fingers[4] == 0) {
		letter = 'W';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 1 && fingers[4] == 0) {
		letter = 'X';
	} else if (fingers[0] == 2 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 0 && fingers[4] == 2) {
		letter = 'Y';
	} else if (fingers[0] == 0 && fingers[1] == 0 && fingers[2] == 0
			&& fingers[3] == 2 && fingers[4] == 0) {
		letter = 'Z';
	} else if (fingers[0] == 2 && fingers[1] == 2 && fingers[2] == 2
			&& fingers[3] == 2 && fingers[4] == 2) {
		letter = ' ';
	} else {
		letter = ' ';
	}

	return letter;
}
