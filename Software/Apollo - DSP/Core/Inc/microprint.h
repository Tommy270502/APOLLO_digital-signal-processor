#ifndef INC_MICROPRINT_H_
#define INC_MICROPRINT_H_

#include <stdint.h>
#include <stdlib.h>
#include "usb_device.h"
#include "usbd_cdc_if.h"

void uprintNum(uint16_t number);
void uprintStr(char*txt);
void uprintFloat(float num);

#endif /* INC_MICROPRINT_H_ */
