#ifndef BOOT_BOOT_H_
#define BOOT_BOOT_H_

#include <stdint.h>

uint32_t LoadApplication(uint8_t index);
void BootToAddress(uint32_t baseAddress);

#endif /* BOOT_BOOT_H_ */
