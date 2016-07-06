#ifndef LIBS_BASE_INCLUDE_BASE_ECC_H_
#define LIBS_BASE_INCLUDE_BASE_ECC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum { EccResultNoError, EccResultCorrected, EccResultNotCorrected, EccResultCorrupted } EccResult;

uint32_t EccCalc(uint8_t* data, uint32_t dataLen);

EccResult EccCorrect(uint32_t generated, uint32_t read, uint8_t* data, uint32_t dataLen);

#ifdef __cplusplus
}
#endif

#endif /* LIBS_BASE_INCLUDE_BASE_ECC_H_ */
