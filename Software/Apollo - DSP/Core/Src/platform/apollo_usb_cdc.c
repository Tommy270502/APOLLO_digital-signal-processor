#include "platform/apollo_usb_cdc.h"

#include <string.h>
#include "usbd_cdc_if.h"

#define APOLLO_USB_RX_BUFFER_SIZE    256U

static volatile uint16_t rx_head;
static volatile uint16_t rx_tail;
static char rx_buffer[APOLLO_USB_RX_BUFFER_SIZE];
static volatile uint32_t rx_overflow;

static uint16_t rx_next(uint16_t index) {
	return (uint16_t)((index + 1U) % APOLLO_USB_RX_BUFFER_SIZE);
}

void apollo_usb_cdc_receive_bytes(const uint8_t *data, uint32_t length) {
	if (data == NULL) {
		return;
	}

	for (uint32_t i = 0U; i < length; i++) {
		uint16_t next = rx_next(rx_head);

		if (next == rx_tail) {
			rx_overflow++;
			break;
		}

		rx_buffer[rx_head] = (char)data[i];
		rx_head = next;
	}
}

apollo_status_t apollo_usb_cdc_read_line(char *line, size_t length) {
	size_t count = 0U;
	uint16_t cursor = rx_tail;

	if (line == NULL || length == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	while (cursor != rx_head) {
		char c = rx_buffer[cursor];
		if (c == '\n' || c == '\r') {
			break;
		}
		cursor = rx_next(cursor);
		count++;
	}

	if (cursor == rx_head) {
		return APOLLO_STATUS_BUSY;
	}

	count = 0U;
	while (rx_tail != cursor && count < (length - 1U)) {
		line[count++] = rx_buffer[rx_tail];
		rx_tail = rx_next(rx_tail);
	}
	line[count] = '\0';

	while (rx_tail != rx_head && (rx_buffer[rx_tail] == '\n' || rx_buffer[rx_tail] == '\r')) {
		rx_tail = rx_next(rx_tail);
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_usb_cdc_write_bytes(const uint8_t *data, uint16_t length) {
	uint8_t result;

	if (data == NULL || length == 0U) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	result = CDC_Transmit_FS((uint8_t *)data, length);
	if (result == USBD_BUSY) {
		return APOLLO_STATUS_BUSY;
	}

	if (result != USBD_OK) {
		return APOLLO_STATUS_HAL_ERROR;
	}

	return APOLLO_STATUS_OK;
}

apollo_status_t apollo_usb_cdc_write(const char *text) {
	if (text == NULL) {
		return APOLLO_STATUS_INVALID_ARG;
	}

	return apollo_usb_cdc_write_bytes((const uint8_t *)text, (uint16_t)strlen(text));
}

uint32_t apollo_usb_cdc_rx_overflow_count(void) {
	return rx_overflow;
}
