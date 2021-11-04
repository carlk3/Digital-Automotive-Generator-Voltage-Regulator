/*
 * console.c
 *
 *  Created on: Nov 3, 2021
 *      Author: carlk
 */

#include <string.h>

#include "main.h"
#include "console.h"

int __io_putchar(int ch) {
//	LL_USART_SetTransferDirection(USART2,  LL_USART_DIRECTION_TX);
//	while (!LL_USART_IsActiveFlag_TEACK(USART2))
//		HAL_Delay(1); // FIXME: osdelay;
//	// Wait until USART Transmit Data Register Empty Flag is set

	while (!LL_USART_IsActiveFlag_TXE(USART2))
		HAL_Delay(1); // FIXME: osdelay

	LL_USART_TransmitData8(USART2, ch);

	while (!LL_USART_IsActiveFlag_TEACK(USART2))
		HAL_Delay(1); // FIXME: osdelay

//	LL_USART_SetTransferDirection(USART2,  LL_USART_DIRECTION_RX);
//	while (!LL_USART_IsActiveFlag_REACK(USART2))
//		HAL_Delay(1); // FIXME: osdelay;
	return ch;
}

void print(const char *str) {
	for (size_t i = 0; i < strlen(str); ++i) {
		__io_putchar(str[i]);
	}
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

		/* Queue the character */
		// ...

		/* Echo received character on TX */

		if (received_char)
			__io_putchar(received_char);
	}
}
