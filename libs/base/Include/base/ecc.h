#ifndef LIBS_BASE_INCLUDE_BASE_ECC_H_
#define LIBS_BASE_INCLUDE_BASE_ECC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @defgroup ECC Error Correction Code utilities
 * @{
 * */

/**
 * ECC correction result
 */
typedef enum {
    /** No error detected */
    EccResultNoError,
    /** Error detected and correccted */
    EccResultCorrected,
    /** Error detected but unable to correct */
    EccResultNotCorrected,
    /** ECC is corrupted */
    EccResultCorrupted
} EccResult;

/**
 * Calculates SEC-DED ECC for given data
 * @param[in] data Data
 * @param[in] dataLen Length of data buffer. Must be power of 2 and greater than 0
 * @return ECC value (3 bytes)
 */
uint32_t EccCalc(uint8_t* const data, uint32_t dataLen);

/**
 * Corrects data by comparing two ECC codes. Error is corrected in-place
 *
 * @param[in] generated ECC generated when reading data
 * @param[in] read ECC stored when writing data
 * @param[inout] data Data
 * @param[in] dataLen Length of data. Must be power of 2 and greater than 0
 * @return Correction result
 */
EccResult EccCorrect(uint32_t generated, uint32_t read, uint8_t* data, uint32_t dataLen);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* LIBS_BASE_INCLUDE_BASE_ECC_H_ */
