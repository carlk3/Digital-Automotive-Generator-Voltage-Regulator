/* dr_spi.c
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

#include <stdbool.h>
#include <stdio.h>
//
#include "hw_config.h"
#include "main.h"
//
#include "dr_spi.h"

#define myASSERT configASSERT
#define DBG_PRINTF printf

static void callback(SPI_HandleTypeDef *phspi) {
    if (phspi->Instance == SPI1) {
        spi_t *pSPI = spi_get_by_hndl(phspi);
        configASSERT(pSPI);
        /* The xHigherPriorityTaskWoken parameter must be initialized to pdFALSE
         as it will get set to pdTRUE inside the interrupt safe API function if
         a context switch is required. */
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        /* Send a notification directly to the task to which interrupt
         processing is being deferred. */
        vTaskNotifyGiveFromISR(pSPI->owner,  // The handle of the task to which
                                             // the notification is being sent.
                               &xHigherPriorityTaskWoken);

        /* Pass the xHigherPriorityTaskWoken value into portYIELD_FROM_ISR().
         If xHigherPriorityTaskWoken was set to pdTRUE inside
         vTaskNotifyGiveFromISR() then calling portYIELD_FROM_ISR() will
         request a context switch. If xHigherPriorityTaskWoken is still
         pdFALSE then calling portYIELD_FROM_ISR() will have no effect. */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *phspi) { callback(phspi); }
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *phspi) { callback(phspi); }
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *phspi) { callback(phspi); }
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    uint32_t err = HAL_SPI_GetError(hspi);
    printf("SPI Error: 0x%lX\r\n", err);
    Error_Handler();
}

// SPI Transfer: Read & Write (simultaneously) on SPI bus
//   If the data that will be received is not important, pass NULL as rx.
//   If the data that will be transmitted is not important,
//     pass NULL as tx and then the SPI_FILL_CHAR is sent out as each data
//     element.
bool spi_transfer(spi_t *pSPI, const uint8_t *tx, uint8_t *rx, size_t length) {
    configASSERT(xTaskGetCurrentTaskHandle() == pSPI->owner);
    configASSERT(tx || rx);
    configASSERT(1 <= length && length <= 512);

    /* Ensure this task does not already have a notification pending by calling
     ulTaskNotifyTake() with the xClearCountOnExit parameter set to pdTRUE, and
     a block time of 0 (don't block). */
    BaseType_t rc = ulTaskNotifyTake(pdTRUE, 0);
    configASSERT(!rc);

    if (tx && !rx) {
        if (HAL_SPI_Transmit_DMA(pSPI->phspi, (uint8_t *)tx, length) != HAL_OK)
            Error_Handler();
    } else if (rx && !tx) {
        //    	/*
        //    	  Master Receive mode restriction:
        //      (#) In Master unidirectional receive-only mode (MSTR =1,
        //      BIDIMODE=0, RXONLY=1) or
        //          bidirectional receive mode (MSTR=1, BIDIMODE=1, BIDIOE=0),
        //          to ensure that the SPI does not initiate a new transfer the
        //          following procedure has to be respected:
        //          (##) HAL_SPI_DeInit()
        //          (##) HAL_SPI_Init()
        //  	 */
        //    	HAL_SPI_DeInit(pSPI->phspi);
        //    	HAL_SPI_Init(pSPI->phspi);
        if (HAL_SPI_Receive_DMA(pSPI->phspi, rx, length) != HAL_OK)
            Error_Handler();
    } else {
        if (HAL_SPI_TransmitReceive_DMA(pSPI->phspi, (uint8_t *)tx, rx,
                                        length) != HAL_OK)
            Error_Handler();
    }
    /* Timeout 1 sec */
    uint32_t timeOut = 1000;
    /* Wait until master completes transfer or time out has occured. */
    rc = ulTaskNotifyTake(
        pdFALSE, pdMS_TO_TICKS(timeOut));  // Wait for notification from ISR
    if (!rc) {
        // This indicates that xTaskNotifyWait() returned without the
        // calling task receiving a task notification. The calling task will
        // have been held in the Blocked state to wait for its notification
        // state to become pending, but the specified block time expired
        // before that happened.
        DBG_PRINTF("Task %s timed out in %s\n",
                   pcTaskGetName(xTaskGetCurrentTaskHandle()), __FUNCTION__);
        return false;
    }
    HAL_SPI_StateTypeDef st = HAL_SPI_GetState(pSPI->phspi);
    switch (st) {
        case HAL_SPI_STATE_RESET:
            printf("Peripheral not Initialized\r\n");
            break;
        case HAL_SPI_STATE_READY: /* printf("Peripheral Initialized and ready
                                     for use\r\n"); */
            break;
        case HAL_SPI_STATE_BUSY:
            printf("an internal process is ongoing\r\n");
            break;
        case HAL_SPI_STATE_BUSY_TX:
            printf("Data Transmission process is ongoing\r\n");
            break;
        case HAL_SPI_STATE_BUSY_RX:
            printf("Data Reception process is ongoing\r\n");
            break;
        case HAL_SPI_STATE_BUSY_TX_RX:
            printf("Data Transmission and Reception process is ongoing\r\n");
            break;
        case HAL_SPI_STATE_ERROR:
            printf("SPI error state\r\n");
            break;
        case HAL_SPI_STATE_ABORT:
            printf("SPI abort is ongoing\r\n");
            break;
    }
    if (!(pSPI->phspi->Instance->SR & SPI_SR_TXE))
        printf("Tx buffer not empty!\r\n");
    if (pSPI->phspi->Instance->SR & SPI_SR_RXNE)
        printf("Rx buffer not empty!\r\n");

    //	HAL_SPI_DMAStop(pSPI->phspi);

    return true;
}

bool my_spi_init(spi_t *pSPI) {
    if (pSPI->initialized) return true;

    // The SPI may be shared (using multiple SSs); protect it
    pSPI->mutex = xSemaphoreCreateRecursiveMutex();

    pSPI->initialized = true;
    return pSPI->initialized;
}

/* [] END OF FILE */
