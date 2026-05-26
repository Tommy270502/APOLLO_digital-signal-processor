#ifndef PLATFORM_APOLLO_USB_CDC_H_
#define PLATFORM_APOLLO_USB_CDC_H_

#include <stddef.h>
#include <stdint.h>
#include "apollo_status.h"

#ifdef __cplusplus
extern "C" {
#endif

void apollo_usb_cdc_receive_bytes(const uint8_t *data, uint32_t length);
apollo_status_t apollo_usb_cdc_read_line(char *line, size_t length);
apollo_status_t apollo_usb_cdc_write(const char *text);
apollo_status_t apollo_usb_cdc_write_bytes(const uint8_t *data, uint16_t length);
uint32_t apollo_usb_cdc_rx_overflow_count(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_APOLLO_USB_CDC_H_ */
