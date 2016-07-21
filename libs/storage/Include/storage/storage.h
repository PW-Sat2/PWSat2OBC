#ifndef LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_
#define LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_

#include <stdint.h>

typedef enum {
    FlashStatusOK,
    FlashStatusInvalidDevice,
    FlashStatusWriteError,
    FlashStatusReadError,
    FlashStatusErrorCorrected,
    FlashStatusErrorNotCorrected,
    FlashStatusChecksumCorrupted
} FlashStatus;

#endif /* LIBS_STORAGE_INCLUDE_STORAGE_STORAGE_H_ */
