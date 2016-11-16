#include "ecc.h"

EccResult EccCorrect(uint32_t generated, uint32_t read, uint8_t* data, uint32_t dataLen)
{
    const uint32_t power = __builtin_ctz(dataLen * 8);
    const uint32_t eccBits = (1 << (2 * power)) - 1;
    const uint32_t correctableMask = (0x55555555 & eccBits);

    const uint32_t syndrome = (generated ^ read) & eccBits;

    if (syndrome == 0)
        return EccResultNoError; /* No errors in data. */

    const uint32_t eccPn = syndrome & correctableMask;       /* Get odd parity bits.  */
    const uint32_t eccP = (syndrome >> 1) & correctableMask; /* Get even parity bits. */

    if ((eccPn ^ eccP) == correctableMask) /* 1-bit correctable error ? */
    {
        const uint8_t bitNum = (eccP & 0x01) | ((eccP >> 1) & 0x02) | ((eccP >> 2) & 0x04);

        const uint32_t byteAddr = ((eccP >> 6) & 0x001) | ((eccP >> 7) & 0x002) | ((eccP >> 8) & 0x004) |
            ((eccP >> 9) & 0x008) | ((eccP >> 10) & 0x010) | ((eccP >> 11) & 0x020) | ((eccP >> 12) & 0x040) |
            ((eccP >> 13) & 0x080) | ((eccP >> 14) & 0x100);

        data[byteAddr] ^= 1 << bitNum;

        return EccResultCorrected;
    }

    /* Count number of one's in the syndrome. */
    uint8_t count = 0;
    uint32_t mask = 0x00800000;
    while (mask)
    {
        if (syndrome & mask)
            count++;
        mask >>= 1;
    }

    if (count == 1) /* Error in the ECC itself. */
        return EccResultCorrupted;

    return EccResultNotCorrected; /* Unable to correct data. */
}

uint32_t EccCalc(uint8_t* const data, uint32_t dataLen)
{
    const uint32_t power = __builtin_ctz(dataLen * 8);

    uint32_t ecc = 0;

    uint32_t p1p = 0;
    uint32_t p1_ = 0;
    uint32_t p2p = 0;
    uint32_t p2_ = 0;
    uint32_t p4p = 0;
    uint32_t p4_ = 0;

    uint32_t p = 0;

    for (uint32_t i = 0; i < dataLen; i++)
    {
        const uint8_t b = data[i];

        p1p = ((b >> 6) & 1) ^ ((b >> 4) & 1) ^ ((b >> 2) & 1) ^ ((b >> 0) & 1) ^ p1p;
        p1_ = ((b >> 7) & 1) ^ ((b >> 5) & 1) ^ ((b >> 3) & 1) ^ ((b >> 1) & 1) ^ p1_;

        p2p = ((b >> 5) & 1) ^ ((b >> 4) & 1) ^ ((b >> 1) & 1) ^ ((b >> 0) & 1) ^ p2p;
        p2_ = ((b >> 7) & 1) ^ ((b >> 6) & 1) ^ ((b >> 3) & 1) ^ ((b >> 2) & 1) ^ p2_;

        p4p = ((b >> 3) & 1) ^ ((b >> 2) & 1) ^ ((b >> 1) & 1) ^ ((b >> 0) & 1) ^ p4p;
        p4_ = ((b >> 7) & 1) ^ ((b >> 6) & 1) ^ ((b >> 5) & 1) ^ ((b >> 4) & 1) ^ p4_;

        const uint8_t rp = __builtin_parity(data[i]);

        for (uint32_t j = 0; j < power - 3; j++)
        {
            if ((i >> j) & 1)
            {
                p ^= rp << (2 * j + 1);
            }
            else
            {
                p ^= rp << (2 * j + 0);
            }
        }
    }

    ecc = (p1p << 0) | (p1_ << 1) | (p2p << 2) | (p2_ << 3) | (p4p << 4) | (p4_ << 5);
    ecc |= (p << 6);

    return ecc;
}
