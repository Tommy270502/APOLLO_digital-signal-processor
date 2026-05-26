#ifndef APP_APOLLO_CONFIG_H_
#define APP_APOLLO_CONFIG_H_

#include <stdint.h>

#define APOLLO_ADC_MAX_CODE                 4095U
#define APOLLO_DAC_MAX_CODE                 4095U

#define APOLLO_DEFAULT_SAMPLE_PERIOD_MS     1U
#define APOLLO_DEFAULT_SAMPLE_PERIOD_S      0.001f
#define APOLLO_DEFAULT_FILTER_CUTOFF_HZ     1000.0f
#define APOLLO_USB_TELEMETRY_PERIOD_MS      20U

#define APOLLO_ADC_TIMEOUT_MS               2U
#define APOLLO_I2C_TIMEOUT_MS               20U
#define APOLLO_SPI_TIMEOUT_MS               20U

#define APOLLO_CLI_LINE_LENGTH              96U
#define APOLLO_TELEMETRY_LINE_LENGTH        128U

#define APOLLO_SRAM_LOG_RECORD_BYTES        8U

#endif /* APP_APOLLO_CONFIG_H_ */
