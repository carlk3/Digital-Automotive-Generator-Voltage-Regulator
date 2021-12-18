#pragma once

//#include "ff.h"
#include "sd_card.h"    

/* Disk Status Bits (DSTATUS) */
#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */
    
#ifdef __cplusplus
extern "C" {
#endif

    size_t sd_get_num();
    sd_card_t *sd_get_by_num(size_t num);
    
    size_t spi_get_num();
    spi_t *spi_get_by_num(size_t num);
    spi_t *spi_get_by_hndl(SPI_HandleTypeDef *phspi);

#ifdef __cplusplus
}
#endif

/* [] END OF FILE */
