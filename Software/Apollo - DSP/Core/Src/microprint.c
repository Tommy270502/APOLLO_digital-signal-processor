#include "microprint.h"

void uprintFloat(float num) {
	char buf[8];
	static char* crnl = "\r\n";

	gcvt(num, 4, buf);

	CDC_Transmit_FS((uint8_t*)buf, strlen(buf));
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
	CDC_Transmit_FS(crnl, strlen(crnl));
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
}

void uprintNum(uint16_t num) {

	static char* crnl = "\r\n";
	char buf[8];

	itoa(num, buf, 10);
	CDC_Transmit_FS((uint8_t*)buf, strlen(buf));
	//HAL_Delay(1);
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
	CDC_Transmit_FS(crnl, strlen(crnl));
	//HAL_Delay(1);
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
}

void uprintStr(char* txt) {
	static char* crnl = "\r\n";
	CDC_Transmit_FS((char*)txt, strlen(txt));
	//HAL_Delay(1);
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
	CDC_Transmit_FS(crnl, strlen(crnl));
	//HAL_Delay(1);
	for(int i = 0;i<5000;i++) {
		asm("NOP");
	}
}

