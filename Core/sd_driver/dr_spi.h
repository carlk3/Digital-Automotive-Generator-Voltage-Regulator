/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
 */
#ifndef _SPI_H_
#define _SPI_H_

#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "task.h"
#include "semphr.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define SPI_FILL_CHAR (0xFF)
//#define XFER_BLOCK_SIZE 512  // Block size supported for SD card is 512 bytes

// "Class" representing SPIs
typedef struct {
    // SPI HW
	SPI_HandleTypeDef *phspi;
	GPIO_TypeDef *miso_port;
    const uint32_t miso_pin;  // SPI MISO pin number for GPIO
    GPIO_TypeDef *mosi_port;
    const uint32_t mosi_pin;
    GPIO_TypeDef *sck_port;
    const uint32_t sck_pin;
//    const uint32_t baud_rate;
    // State variables:
//    uint32_t tx_dma;
//    uint32_t rx_dma;
//    dma_channel_config tx_dma_cfg;
//    dma_channel_config rx_dma_cfg;
//    irq_handler_t dma_isr;
    bool initialized;         // Assigned dynamically
    TaskHandle_t owner;       // Assigned dynamically
    SemaphoreHandle_t mutex;  // Assigned dynamically
} spi_t;

#ifdef __cplusplus
extern "C" {
#endif

    bool spi_transfer(spi_t *pSPI, const uint8_t *tx, uint8_t *rx, size_t length);
    bool my_spi_init(spi_t *pSPI);

#ifdef __cplusplus
}
#endif

#endif
/* [] END OF FILE */
