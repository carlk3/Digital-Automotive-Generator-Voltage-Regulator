/*

This file should be tailored to match the hardware design.

There should be one element of the spi[] array for each hardware SPI used.

There should be one element of the sd_cards[] array for each SD card slot.
The name is should correspond to the FatFs "logical drive" identifier.
(See http://elm-chan.org/fsw/ff/doc/filename.html#vol)
The rest of the constants will depend on the type of
socket, which SPI it is driven by, and how it is wired.

*/

#include <string.h>
//
//#include "my_debug.h"
//
#include "spi.h"

#include "main.h"
#include "hw_config.h"
//
//#include "ff.h" /* Obtains integer types */
//
//#include "diskio.h" /* Declarations of disk functions */

void spi0_dma_isr();

// Hardware Configuration of SPI "objects"
// Note: multiple SD cards can be driven by one SPI if they use different slave
// selects.
static spi_t spis[] = {  // One for each SPI.
    {
		.phspi = &hspi1,  // handle to SPI
		.miso_port = GPIOB,
        .miso_pin = SPI1_MISO_Pin,
		.mosi_port = GPIOB,
        .mosi_pin = SPI1_MOSI_Pin,
		.sck_port = GPIOB,
        .sck_pin = SPI1_SCK_Pin,
        // Following attributes are dynamically assigned
        .initialized = false,  // initialized flag
        .owner = 0,	// Owning task, assigned dynamically
        .mutex = 0	// Guard semaphore, assigned dynamically
    }};

// Hardware Configuration of the SD Card "objects"
static sd_card_t sd_cards[] = {  // One for each SD card
    {
        .pcName = "0:",           // Name used to mount device
        .spi = &spis[0],          // Pointer to the SPI driving this card
		.ss_port = D10___SPI_SS_GPIO_Port,
        .ss_pin = D10___SPI_SS_Pin,            // The SPI slave select GPIO for this SD card
		.use_card_detect = false,
//		.card_detect_port = D9___Card_Detect_GPIO_Port,
//        .card_detect_pin = D9___Card_Detect_Pin,   // Card detect
        .card_detected_true = 1,  // What the GPIO read returns when a card is
                                  // present. Use -1 if there is no card detect.
        // Following attributes are dynamically assigned
        .m_Status = STA_NOINIT,
        .sectors = 0,
        .card_type = 0,
    }
};

/* ********************************************************************** */
size_t sd_get_num() { return sizeof sd_cards / sizeof sd_cards[0]; }
sd_card_t *sd_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &sd_cards[num];
    } else {
        return NULL;
    }
}
size_t spi_get_num() { return sizeof spis / sizeof spis[0]; }
spi_t *spi_get_by_num(size_t num) {
    if (num <= sd_get_num()) {
        return &spis[num];
    } else {
        return NULL;
    }
}
spi_t *spi_get_by_hndl(SPI_HandleTypeDef *phspi) {
	for (size_t i = 0; i < spi_get_num(); ++i)
		if (spis[i].phspi == phspi) return &spis[i];
	return NULL;
}
/* [] END OF FILE */
