/*
 * config.c
 *
 *  Created on: Jan 2, 2022
 *      Author: carlk
 */

#include <string.h>

#include "main.h"
#include "config.h"

typedef struct {
	config_t data;
	uint8_t pad[FLASH_PAGE_SIZE - sizeof(config_t)];
} flash_config_t __attribute__ ((aligned (FLASH_PAGE_SIZE)));

static const flash_config_t flash_config; // Will be placed in Flash

/* Pad to multiple of quadword size */
// Round up size to multiple of quadword:
#define config_qw_sz ( ((sizeof(config_t) + sizeof(uint64_t) - 1) / sizeof(uint64_t)) * sizeof(uint64_t) )
typedef struct {
	config_t data;
	uint8_t pad[config_qw_sz - sizeof(config_t)];
} ram_config_t;

static ram_config_t ram_config =
{
	{
		// Populate with defaults
		.vlim = 7.5,
		.clim = 45,
		.plim = 180,
		.Bplus_volt_scale = 6.87f / 34060,
		.Bplus_amp_scale = 5.0f/(13845-9961),
		.Bplus_zero = 9961
	}
};

float cfg_get_vlim() {
	return ram_config.data.vlim;
}
void cfg_set_vlim(float f) {
	ram_config.data.vlim = f;
}

float cfg_get_clim() {
	return ram_config.data.clim;
}
void cfg_set_clim(float f) {
	ram_config.data.clim = f;
}

float cfg_get_plim() {
	return ram_config.data.plim;
}
void cfg_set_plim(float f) {
	ram_config.data.plim = f;
}

float cfg_get_Bplus_volt_scale() {
	return ram_config.data.Bplus_volt_scale;
}
void cfg_set_Bplus_volt_scale(float f) {
	ram_config.data.Bplus_volt_scale = f;
}

float cfg_get_Bplus_amp_scale() {
	return ram_config.data.Bplus_amp_scale;
}
void cfg_set_Bplus_amp_scale(float f) {
	ram_config.data.Bplus_amp_scale = f;
}

uint16_t cfg_get_Bplus_amp_zero() {
	return ram_config.data.Bplus_zero;
}
void cfg_set_Bplus_amp_zero(uint16_t u) {
	ram_config.data.Bplus_zero = u;
}

// See STM32Cube/Repository/STM32Cube_FW_L4_V1.17.1/Projects/NUCLEO-L412KB/Examples/FLASH/FLASH_EraseProgram/Src/main.c

/**
 * @brief  Gets the page of a given address
 * @param  Addr: Address of the FLASH Memory
 * @retval The page of a given address
 */
static uint32_t GetPage(uint32_t Addr) {
	uint32_t page = 0;

	if (Addr < (FLASH_BASE + FLASH_BANK_SIZE)) {
		/* Bank 1 */
		page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
	} else {
		/* Bank 2 */
		page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
	}

	return page;
}

static inline uint32_t calculate_checksum(uint32_t const *p, size_t const size){
	uint32_t checksum = 0;
	for (uint32_t i = 0; i < (size/sizeof(uint32_t))-1; i++){
		checksum ^= *p;
		p++;
	}
	return checksum;
}

void cfg_save() {
	configASSERT(FLASH_BASE <= (uint32_t)&flash_config
			&& (uint32_t)&flash_config + sizeof flash_config - 1 <= FLASH_END);

	ram_config.data.signature = 0xBABEBABE;
	ram_config.data.checksum = calculate_checksum((uint32_t *)&ram_config.data, offsetof(config_t, checksum));

	uint32_t page = GetPage((uint32_t) &flash_config);

	/*Variable used for Erase procedure*/
	static FLASH_EraseInitTypeDef EraseInitStruct;

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks = FLASH_BANK_1; // The only option
	EraseInitStruct.Page = page;
	EraseInitStruct.NbPages = 1;

	uint32_t PAGEError = 0;

	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();

//    /* Clear error programming flags */
	uint32_t error = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
    __HAL_FLASH_CLEAR_FLAG(error);

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
		Error_Handler();

	uint32_t src_addr = (uint32_t) &ram_config;
	uint32_t dst_addr = (uint32_t) &flash_config;

	while (src_addr < (uint32_t)&ram_config + sizeof ram_config - 1 ) {
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, dst_addr, *(uint64_t *)src_addr)
				== HAL_OK) {
			src_addr += 8;
			dst_addr += 8;
		} else {
			Error_Handler();
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
}
// Returns false if no valid previous configuration is found
bool cfg_load() {
	config_t const volatile *p_cfg = &flash_config.data;
	// If the compiler doesn't know flash_config is volatile,
	// it optimizes out the following checks (and presumes they always fail):
	if (0xBABEBABE == p_cfg->signature
	&& calculate_checksum((uint32_t *)p_cfg, offsetof(config_t, checksum)) == p_cfg->checksum) {
		memcpy(&ram_config.data, &flash_config.data, sizeof ram_config.data);
		return true;
	}
	// Populated with defaults
	return false;
}
