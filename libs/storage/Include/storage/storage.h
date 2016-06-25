#ifndef LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_
#define LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_

#include <stdint.h>

typedef enum { FlashStatusOK, FlashStatusInvalidDevice, FlashStatusWriteError } FlashStatus;

typedef struct _FlashInterface
{
    void* extra;
    uint32_t baseAddress;
    int (*initialize)(struct _FlashInterface* flash);
    int (*check)(struct _FlashInterface* flash);
    int (*status)(struct _FlashInterface* flash);
    FlashStatus (*readPage)(struct _FlashInterface* interface, uint32_t address, uint8_t* buffer, uint16_t len);
    FlashStatus (*eraseBlock)(struct _FlashInterface* interface, uint32_t address);
    int (*writeBlock)(struct _FlashInterface* interface, uint32_t address, uint8_t value);
    FlashStatus (*writePage)(
        struct _FlashInterface* interface, uint8_t volatile* address, const uint8_t* buffer, uint32_t length);
    uint8_t (*isBadBlock)(const struct _FlashInterface* flash, uint8_t volatile* address);
} FlashInterface;

void DoThings(FlashInterface* flash);

#endif /* LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_ */
