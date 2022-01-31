/*
 * console.c
 *
 *  Created on: Nov 3, 2021
 *      Author: carlk
 */

#include <string.h>

#include "main.h"
#include "console.h"
#include "freertos.h"

#include "printf.h"

int __io_putchar(int ch) {
	// Wait until USART Transmit Data Register Empty Flag is set
	while (!LL_USART_IsActiveFlag_TXE(USART2))
		osDelay(1);
	LL_USART_TransmitData8(USART2, ch);
	return ch;
}

// For Marco Paland's printf
void _putchar(char character) {
	if ('\n' == character)
		__io_putchar('\r');
	__io_putchar(character);
}

/**
 * @brief  Function called from USART IRQ Handler when RXNE flag is set
 *         Function is in charge of reading character received on USART RX line.
 * @param  None
 * @retval None
 */
void USART_CharReception_Callback() {

	while (LL_USART_IsActiveFlag_RXNE(USART2)) {
		__IO uint32_t received_char = 0;

		/* Read Received character. RXNE flag is cleared by reading of RDR register */
		received_char = LL_USART_ReceiveData8(USART2);

//		/* Echo received character on TX */
//		if (received_char)
//			__io_putchar(received_char);
//
		/* Queue the character */
		evt_t evt =
			{ KEYSTROKE_SIG, .content.data = received_char };
		osStatus_t rc = osMessageQueuePut(ConsoleEvtQHandle, &evt, 0, 0);
		if (osOK != rc)
			printf("\nKeystroke overrun!\n");
	}
}
