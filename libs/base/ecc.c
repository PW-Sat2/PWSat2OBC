#include "ecc.h"

EccResult EccCorrect(uint32_t generated, uint32_t read, uint8_t* data, uint32_t dataLen)
{
    uint32_t power = __builtin_ctz(dataLen * 8);
    uint32_t eccBits = (1 << (2 * power)) - 1;
    uint32_t correctableMask = (0x55555555 & eccBits);

    int count, bitNum, byteAddr;
    uint32_t mask;
    uint32_t syndrome;
    uint32_t eccP;  /* even ECC parity bits. */
    uint32_t eccPn; /* odd ECC parity bits.  */

    syndrome = (generated ^ read) & eccBits;

    if (syndrome == 0)
        return EccResultNoError; /* No errors in data. */

    eccPn = syndrome & correctableMask;       /* Get odd parity bits.  */
    eccP = (syndrome >> 1) & correctableMask; /* Get even parity bits. */

    if ((eccPn ^ eccP) == correctableMask) /* 1-bit correctable error ? */
    {
        bitNum = (eccP & 0x01) | ((eccP >> 1) & 0x02) | ((eccP >> 2) & 0x04);

        byteAddr = ((eccP >> 6) & 0x001) | ((eccP >> 7) & 0x002) | ((eccP >> 8) & 0x004) | ((eccP >> 9) & 0x008) |
            ((eccP >> 10) & 0x010) | ((eccP >> 11) & 0x020) | ((eccP >> 12) & 0x040) | ((eccP >> 13) & 0x080) |
            ((eccP >> 14) & 0x100);

        data[byteAddr] ^= 1 << bitNum;

        return EccResultCorrected;
    }

    /* Count number of one's in the syndrome. */
    count = 0;
    mask = 0x00800000;
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

uint32_t EccCalc(const uint8_t* data, uint32_t dataLen)
{
    uint32_t power = __builtin_ctz(dataLen * 8);

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
        uint8_t b = data[i];

        p1p = ((b >> 6) & 1) ^ ((b >> 4) & 1) ^ ((b >> 2) & 1) ^ ((b >> 0) & 1) ^ p1p;
        p1_ = ((b >> 7) & 1) ^ ((b >> 5) & 1) ^ ((b >> 3) & 1) ^ ((b >> 1) & 1) ^ p1_;

        p2p = ((b >> 5) & 1) ^ ((b >> 4) & 1) ^ ((b >> 1) & 1) ^ ((b >> 0) & 1) ^ p2p;
        p2_ = ((b >> 7) & 1) ^ ((b >> 6) & 1) ^ ((b >> 3) & 1) ^ ((b >> 2) & 1) ^ p2_;

        p4p = ((b >> 3) & 1) ^ ((b >> 2) & 1) ^ ((b >> 1) & 1) ^ ((b >> 0) & 1) ^ p4p;
        p4_ = ((b >> 7) & 1) ^ ((b >> 6) & 1) ^ ((b >> 5) & 1) ^ ((b >> 4) & 1) ^ p4_;

        uint8_t rp = __builtin_parity(data[i]);

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
