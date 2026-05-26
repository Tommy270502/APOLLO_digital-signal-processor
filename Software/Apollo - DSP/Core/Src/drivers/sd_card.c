#include "drivers/sd_card.h"

apollo_status_t sd_card_init(SPI_HandleTypeDef *spi) {
	(void)spi;
	return APOLLO_STATUS_UNSUPPORTED;
}

apollo_status_t sd_card_status(void) {
	return APOLLO_STATUS_UNSUPPORTED;
}
