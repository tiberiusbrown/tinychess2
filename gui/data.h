#ifndef DATA_H
#define DATA_H

#include <ALLDATA_COMPRESSED.h>

#include "common.h"
#include "config.h"

static uint8_t const* uncompress_p = NULL;
uint8_t uncompress_m = 0;

static FORCEINLINE uint32_t read_one(void)
{
    uint32_t r = (*uncompress_p & uncompress_m) != 0;
    if(!(uncompress_m <<= 1))
    {
        ++uncompress_p;
        uncompress_m = 0x1;
    }
    return r;
}

static uint32_t read_n(int n)
{
    uint32_t r = 0;
    while(n-- > 0)
        r = (r << 1) | read_one();
    return r;
}

static FORCEINLINE void uncompress_data(void)
{
    uint8_t* p;

    uncompress_p = ALLDATA_COMPRESSED;
    uncompress_m = 0x01;

    p = ALLDATA_UNCOMPRESSED;

    while(p < ALLDATA_UNCOMPRESSED + sizeof(ALLDATA_UNCOMPRESSED))
    {
        if(read_n(1))
        {
            uint32_t j = 0;
            /* offset+length */
#if 0
            uint32_t offset = read_n(COMPRESS_OFFSET_BITS);
            uint32_t length = read_n(COMPRESS_LENGTH_BITS) + 2;
#else
            /* two bytes smaller code size :P */
            uint32_t offset = read_n(COMPRESS_OFFSET_BITS + COMPRESS_LENGTH_BITS);
            uint32_t length = (offset & ((1 << COMPRESS_LENGTH_BITS) - 1)) + 2;
            offset >>= COMPRESS_LENGTH_BITS;
#endif
            offset += length;
            while(j < length)
            {
                *p = *(p - offset);
                ++j;
                ++p;
            }
        }
        else
        {
            /* literal */
            *p++ = (uint8_t)read_n(8);
        }
    }
}

#endif
