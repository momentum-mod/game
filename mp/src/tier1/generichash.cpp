//======= Copyright 2005-2011, Valve Corporation, All rights reserved. =========
//
// Public domain MurmurHash3 by Austin Appleby is a very solid general-purpose
// hash with a 32-bit output. References:
// http://code.google.com/p/smhasher/ (home of MurmurHash3)
// https://sites.google.com/site/murmurhash/avalanche
// http://www.strchr.com/hash_functions
//
// Variant Pearson Hash general purpose hashing algorithm described
// by Cargill in C++ Report 1994. Generates a 16-bit result.
// Now relegated to PearsonHash namespace, not recommended for use
//
//=============================================================================

#if !defined(_MINIMUM_BUILD_)
#include <stdlib.h>
#include <tier0/basetypes.h>
#include <tier0/platform.h>
#include <tier1/generichash.h>
#include <tier1/strtools.h>
#else
#include <tier0/basetypes.h>
#include <tier1/generichash.h>

// in platform.h...
#define LittleDWord(val) (val)
#endif

#if defined(_MSC_VER)

#define ROTL32(x, y) _rotl(x, y)
#define ROTL64(x, y) _rotl64(x, y)

#define BIG_CONSTANT(x) (x)

#else // defined(_MSC_VER)

static inline uint32 rotl32(uint32 x, int8 r) { return (x << r) | (x >> (32 - r)); }

static inline uint64 rotl64(uint64 x, int8 r) { return (x << r) | (x >> (64 - r)); }

#define ROTL32(x, y) rotl32(x, y)
#define ROTL64(x, y) rotl64(x, y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------

uint32 MurmurHash3_32(const void *key, size_t len, uint32 seed, bool bCaselessStringVariant)
{
    const uint8 *data = (const uint8 *)key;
    const ptrdiff_t nblocks = len / 4;
    uint32 uSourceBitwiseAndMask = 0xDFDFDFDF | ((uint32)bCaselessStringVariant - 1);

    uint32 h1 = seed;

    //----------
    // body

    const uint32 *blocks = (const uint32 *)(data + nblocks * 4);

    for (ptrdiff_t i = -nblocks; i; i++)
    {
        uint32 k1 = LittleDWord(blocks[i]);
        k1 &= uSourceBitwiseAndMask;

        k1 *= 0xcc9e2d51;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= 0x1b873593;

        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xe6546b64;
    }

    //----------
    // tail

    const uint8 *tail = (const uint8 *)(data + nblocks * 4);

    uint32 k1 = 0;

    switch (len & 3)
    {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];
        k1 &= uSourceBitwiseAndMask;
        k1 *= 0xcc9e2d51;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= 0x1b873593;
        h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;

    h1 ^= h1 >> 16;
    h1 *= 0x85ebca6b;
    h1 ^= h1 >> 13;
    h1 *= 0xc2b2ae35;
    h1 ^= h1 >> 16;

    return h1;
}

static inline uint64 fmix64(uint64 k)
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

void MurmurHash3_128(const void *key, const int len, const uint32 seed, void *out)
{
    const uint8 *data = (const uint8 *)key;
    const int nblocks = len / 16;

    uint64 h1 = seed;
    uint64 h2 = seed;

    const uint64 c1 = BIG_CONSTANT(0x87c37b91114253d5);
    const uint64 c2 = BIG_CONSTANT(0x4cf5ad432745937f);

    //----------
    // body

    const uint64 *blocks = (const uint64 *)(data);

    for (int i = 0; i < nblocks; i++)
    {
        uint64 k1 = blocks[i * 2 + 0];
        uint64 k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1 = ROTL64(k1, 31);
        k1 *= c2;
        h1 ^= k1;

        h1 = ROTL64(h1, 27);
        h1 += h2;
        h1 = h1 * 5 + 0x52dce729;

        k2 *= c2;
        k2 = ROTL64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

        h2 = ROTL64(h2, 31);
        h2 += h1;
        h2 = h2 * 5 + 0x38495ab5;
    }

    //----------
    // tail

    const uint8 *tail = (const uint8 *)(data + nblocks * 16);

    uint64 k1 = 0;
    uint64 k2 = 0;

    switch (len & 15)
    {
    case 15:
        k2 ^= ((uint64)tail[14]) << 48;
    case 14:
        k2 ^= ((uint64)tail[13]) << 40;
    case 13:
        k2 ^= ((uint64)tail[12]) << 32;
    case 12:
        k2 ^= ((uint64)tail[11]) << 24;
    case 11:
        k2 ^= ((uint64)tail[10]) << 16;
    case 10:
        k2 ^= ((uint64)tail[9]) << 8;
    case 9:
        k2 ^= ((uint64)tail[8]) << 0;
        k2 *= c2;
        k2 = ROTL64(k2, 33);
        k2 *= c1;
        h2 ^= k2;

    case 8:
        k1 ^= ((uint64)tail[7]) << 56;
    case 7:
        k1 ^= ((uint64)tail[6]) << 48;
    case 6:
        k1 ^= ((uint64)tail[5]) << 40;
    case 5:
        k1 ^= ((uint64)tail[4]) << 32;
    case 4:
        k1 ^= ((uint64)tail[3]) << 24;
    case 3:
        k1 ^= ((uint64)tail[2]) << 16;
    case 2:
        k1 ^= ((uint64)tail[1]) << 8;
    case 1:
        k1 ^= ((uint64)tail[0]) << 0;
        k1 *= c1;
        k1 = ROTL64(k1, 31);
        k1 *= c2;
        h1 ^= k1;
    };

    //----------
    // finalization

    h1 ^= len;
    h2 ^= len;

    h1 += h2;
    h2 += h1;

    h1 = fmix64(h1);
    h2 = fmix64(h2);

    h1 += h2;
    h2 += h1;

    ((uint64 *)out)[0] = h1;
    ((uint64 *)out)[1] = h2;
}

#if !defined(_MINIMUM_BUILD_)

namespace PearsonHash
{

//-----------------------------------------------------------------------------
//
// Table of randomly shuffled values from 0-255 generated by:
//
//-----------------------------------------------------------------------------
/*
void MakeRandomValues()
{
    int i, j, r;
    unsigned  t;
    srand( 0xdeadbeef );

    for ( i = 0; i < 256; i++ )
    {
        g_nRandomValues[i] = (unsigned )i;
    }

    for (j = 0; j < 8; j++)
    {
        for (i = 0; i < 256; i++)
        {
            r = rand() & 0xff;
            t = g_nRandomValues[i];
            g_nRandomValues[i] = g_nRandomValues[r];
            g_nRandomValues[r] = t;
        }
    }

    printf("static unsigned g_nRandomValues[256] =\n{\n");

    for (i = 0; i < 256; i += 16)
    {
        printf("\t");
        for (j = 0; j < 16; j++)
            printf(" %3d,", g_nRandomValues[i+j]);
        printf("\n");
    }
    printf("};\n");
}
*/

static const unsigned char g_nRandomValues[256] = {
    131, 184, 146, 42,  124, 142, 226, 76,  8,   135, 215, 116, 228, 49,  144, 231, 238, 25,  156, 125, 128, 87,
    223, 38,  98,  122, 105, 4,   35,  180, 188, 160, 179, 59,  218, 0,   192, 207, 209, 150, 227, 143, 140, 161,
    73,  84,  111, 162, 239, 74,  210, 195, 15,  225, 104, 221, 245, 12,  72,  47,  109, 187, 40,  178, 208, 56,
    190, 193, 126, 95,  33,  45,  177, 170, 186, 123, 202, 149, 60,  194, 168, 102, 71,  148, 46,  121, 52,  119,
    196, 247, 127, 145, 75,  79,  61,  254, 9,   44,  23,  211, 18,  175, 251, 130, 203, 108, 85,  167, 29,  250,
    138, 182, 101, 213, 159, 92,  36,  10,  86,  32,  176, 80,  17,  134, 181, 114, 64,  165, 89,  68,  6,   14,
    205, 137, 117, 7,   39,  132, 26,  19,  214, 99,  166, 163, 69,  174, 157, 100, 201, 118, 2,   28,  235, 236,
    139, 244, 70,  20,  155, 82,  51,  154, 115, 94,  93,  83,  136, 27,  198, 43,  50,  243, 183, 153, 53,  206,
    77,  55,  57,  3,   220, 147, 253, 110, 37,  246, 97,  13,  120, 103, 91,  169, 58,  11,  133, 22,  152, 189,
    222, 151, 141, 88,  224, 1,   48,  191, 249, 173, 106, 113, 252, 172, 232, 66,  219, 96,  237, 21,  233, 62,
    242, 54,  230, 65,  78,  248, 16,  41,  31,  200, 90,  112, 255, 171, 164, 24,  199, 81,  212, 197, 185, 67,
    5,   234, 30,  129, 216, 63,  204, 158, 217, 229, 107, 240, 241, 34,
};

//-----------------------------------------------------------------------------
// String
//-----------------------------------------------------------------------------
unsigned FASTCALL HashString(const char *pszKey)
{
    const uint8 *k = (const uint8 *)pszKey;
    uint8 even = 0, odd = 0, n;

    while ((n = *k++) != 0)
    {
        even = g_nRandomValues[even ^ n];
        if ((n = *k++) != 0)
            odd = g_nRandomValues[odd ^ n];
        else
            break;
    }

    return ((unsigned int)even << 8) | odd;
}

#define InlineToUpper(c) (((c >= 'a') && (c <= 'z')) ? (c - 0x20) : c)

//-----------------------------------------------------------------------------
// Case-insensitive string
//-----------------------------------------------------------------------------
unsigned FASTCALL HashStringCaseless(const char *pszKey)
{
    const uint8 *k = (const uint8 *)pszKey;
    uint8 even = 0, odd = 0, n;

    while ((n = *k++) != 0)
    {
        even = g_nRandomValues[even ^ InlineToUpper(n)];

        if ((n = *k++) != 0)
            odd = g_nRandomValues[odd ^ InlineToUpper(n)];
        else
            break;
    }

    return ((unsigned int)even << 8) | odd;
}

unsigned FASTCALL Hash8(const void *pKey)
{
    const uint32 *p = (const uint32 *)pKey;
    unsigned even, odd, n;
    n = *p;
    even = g_nRandomValues[n & 0xff];
    odd = g_nRandomValues[((n >> 8) & 0xff)];

    even = g_nRandomValues[odd ^ (n >> 24)];
    odd = g_nRandomValues[even ^ ((n >> 16) & 0xff)];
    even = g_nRandomValues[odd ^ ((n >> 8) & 0xff)];
    odd = g_nRandomValues[even ^ (n & 0xff)];

    n = *(p + 1);
    even = g_nRandomValues[odd ^ (n >> 24)];
    odd = g_nRandomValues[even ^ ((n >> 16) & 0xff)];
    even = g_nRandomValues[odd ^ ((n >> 8) & 0xff)];
    odd = g_nRandomValues[even ^ (n & 0xff)];

    return (even << 8) | odd;
}

} // namespace PearsonHash
#endif // _MINIMUM_BUILD_