/* sd_spi.c
Copyright 2021 Carl John Kugler III

Licensed under the Apache License, Version 2.0 (the License); you may not use 
this file except in compliance with the License. You may obtain a copy of the 
License at

   http://www.apache.org/licenses/LICENSE-2.0 
Unless required by applicable law or agreed to in writing, software distributed 
under the License is distributed on an AS IS BASIS, WITHOUT WARRANTIES OR 
CONDITIONS OF ANY KIND, either express or implied. See the License for the 
specific language governing permissions and limitations under the License.
*/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <string.h> // memset
//
//#include "my_debug.h"
#include "main.h"
#include "sd_card.h"
#include "dr_spi.h"

#include "sd_spi.h"
#include "spi.h"

//#define TRACE_PRINTF(fmt, args...)
#define TRACE_PRINTF printf  // task_printf

void sd_spi_go_high_frequency(sd_card_t *pSD) {
	__HAL_SPI_DISABLE(pSD->spi->phspi);
	pSD->spi->phspi->Instance->CR1 &= ~SPI_CR1_BR_Msk;
//	pSD->spi->phspi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_16;
	pSD->spi->phspi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_64;
	__HAL_SPI_ENABLE(pSD->spi->phspi);
}
void sd_spi_go_low_frequency(sd_card_t *pSD) {
	__HAL_SPI_DISABLE(pSD->spi->phspi);
	pSD->spi->phspi->Instance->CR1 &= ~SPI_CR1_BR_Msk;
	pSD->spi->phspi->Instance->CR1 |= SPI_BAUDRATEPRESCALER_256;
	__HAL_SPI_ENABLE(pSD->spi->phspi);
}

static void sd_spi_lock(sd_card_t *pSD) {
    configASSERT(pSD->spi->mutex);
    xSemaphoreTake(pSD->spi->mutex, portMAX_DELAY);
    configASSERT(0 == pSD->spi->owner);    
    pSD->spi->owner = xTaskGetCurrentTaskHandle();
}
static void sd_spi_unlock(sd_card_t *pSD) {
    configASSERT(xTaskGetCurrentTaskHandle() == pSD->spi->owner);
    pSD->spi->owner = 0;
    xSemaphoreGive(pSD->spi->mutex);
}

// Would do nothing if pSD->ss_gpio were set to GPIO_FUNC_SPI.
static void sd_spi_select(sd_card_t *pSD) {
//    gpio_put(this->ss_gpio, 0);
	HAL_GPIO_WritePin(D10___SPI_SS_GPIO_Port, D10___SPI_SS_Pin, GPIO_PIN_RESET);
    // A fill byte seems to be necessary, sometimes:
    uint8_t fill = SPI_FILL_CHAR;
//    spi_write_blocking(this->spi->hw_inst, &fill, 1);
    sd_spi_write(pSD, fill);
}
static void sd_spi_deselect(sd_card_t *pSD) {
//    gpio_put(this->ss_gpio, 1);
	HAL_GPIO_WritePin(D10___SPI_SS_GPIO_Port, D10___SPI_SS_Pin, GPIO_PIN_SET);
    /*
    MMC/SDC enables/disables the DO output in synchronising to the SCLK. This
    means there is a posibility of bus conflict with MMC/SDC and another SPI
    slave that shares an SPI bus. Therefore to make MMC/SDC release the MISO
    line, the master device needs to send a byte after the CS signal is
    deasserted.
    */
    uint8_t fill = SPI_FILL_CHAR;
//    spi_write_blocking(this->spi->hw_inst, &fill, 1);
    sd_spi_write(pSD, fill);
}

void sd_spi_acquire(sd_card_t *pSD) {
    sd_spi_lock(pSD);
    sd_spi_select(pSD);
}

void sd_spi_release(sd_card_t *pSD) {
    sd_spi_deselect(pSD);
    sd_spi_unlock(pSD);
}

bool sd_spi_transfer(sd_card_t *pSD, const uint8_t *tx, uint8_t *rx,
                     size_t length) {
	if (!tx) memset(rx, SPI_FILL_CHAR, length); // Dummy data on Tx line comes from rx buffer
    return spi_transfer(pSD->spi, tx, rx, length);
}

uint8_t sd_spi_write(sd_card_t *pSD, const uint8_t value) {
    // TRACE_PRINTF("%s\n", __FUNCTION__);
    u_int8_t received = SPI_FILL_CHAR;
    configASSERT(xTaskGetCurrentTaskHandle() == pSD->spi->owner);
#if 0
    if (HAL_SPI_TransmitReceive(pSD->spi->phspi, (uint8_t *)&value, &received, 1, HAL_MAX_DELAY) != HAL_OK)
    	Error_Handler();
#else
    bool success = spi_transfer(pSD->spi, &value, &received, 1);
    configASSERT(success);
#endif
    return received;
}

void sd_spi_send_initializing_sequence(sd_card_t * pSD) {
//    bool old_ss = gpio_get(pSD->ss_gpio);
	GPIO_PinState old_ss = HAL_GPIO_ReadPin(pSD->ss_port, pSD->ss_pin);
    // Set DI and CS high and apply 74 or more clock pulses to SCLK:
//    gpio_put(pSD->ss_gpio, 1);
	HAL_GPIO_WritePin(pSD->ss_port, pSD->ss_pin, 1);
    uint8_t ones[10];
    memset(ones, 0xFF, sizeof ones);
    TickType_t xStart = xTaskGetTickCount();
    do {
        sd_spi_transfer(pSD, ones, NULL, sizeof ones);
    } while ((xTaskGetTickCount() - xStart) < pdMS_TO_TICKS(1));
    HAL_GPIO_WritePin(pSD->ss_port, pSD->ss_pin, old_ss);
}
/* [] END OF FILE */
