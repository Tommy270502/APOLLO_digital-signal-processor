#ifndef APP_APOLLO_APP_H_
#define APP_APOLLO_APP_H_

#include <stdint.h>
#include "apollo_status.h"
#include "app/apollo_cli.h"
#include "app/apollo_diagnostics.h"
#include "app/apollo_signal_chain.h"
#include "app/apollo_storage.h"
#include "drivers/dac_driver.h"
#include "drivers/sram_23k256.h"
#include "platform/apollo_adc.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	ADC_HandleTypeDef *adc;
	I2C_HandleTypeDef *dac_i2c;
	SPI_HandleTypeDef *sram_spi;
	UART_HandleTypeDef *uart;
} apollo_app_handles_t;

typedef struct {
	apollo_adc_t adc;
	dac_driver_t dac;
	sram_23k256_t sram;
	apollo_storage_t storage;
	apollo_signal_chain_t signal_chain;
	apollo_diagnostics_t diagnostics;
	uint32_t sequence;
	uint32_t last_sample_tick;
	uint32_t last_telemetry_tick;
	uint8_t initialized;
	uint8_t telemetry_enabled;
} apollo_app_t;

apollo_status_t apollo_app_init(apollo_app_t *app, const apollo_app_handles_t *handles);
void apollo_app_task(apollo_app_t *app);

#ifdef __cplusplus
}
#endif

#endif /* APP_APOLLO_APP_H_ */
