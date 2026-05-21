#include "microprint.h"

#include <stdio.h>
#include <string.h>

static void usb_print_delay(void) {
	for (int i = 0; i < 5000; i++) {
		asm("NOP");
	}
}

static void usb_write_line(const char *text) {
	static const char crnl[] = "\r\n";

	(void)CDC_Transmit_FS((uint8_t *)text, (uint16_t)strlen(text));
	usb_print_delay();
	(void)CDC_Transmit_FS((uint8_t *)crnl, (uint16_t)strlen(crnl));
	usb_print_delay();
}

void uprintFloat(float num) {
	char buf[16];

	(void)snprintf(buf, sizeof(buf), "%.4g", (double)num);
	usb_write_line(buf);
}

void uprintNum(uint16_t num) {
	char buf[8];

	(void)snprintf(buf, sizeof(buf), "%u", (unsigned int)num);
	usb_write_line(buf);
}

void uprintStr(const char *txt) {
	if (txt != NULL) {
		usb_write_line(txt);
	}
}
