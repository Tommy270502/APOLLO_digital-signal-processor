#ifndef DRIVERS_SD_CARD_H_
#define DRIVERS_SD_CARD_H_

#include "apollo_status.h"
#include "stm32f4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

apollo_status_t sd_card_init(SPI_HandleTypeDef *spi);
apollo_status_t sd_card_status(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SD_CARD_H_ */
