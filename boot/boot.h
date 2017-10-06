#ifndef BOOT_BOOT_H_
#define BOOT_BOOT_H_

#include <stdint.h>

void BootToAddress(uint32_t baseAddress);

void ProceedWithBooting();

void CountBootAttempts();

#endif /* BOOT_BOOT_H_ */
