#ifndef BYTE_ORDER_H
#define BYTE_ORDER_H

#define COMPILED_ON_LITTLE_ENDIAN

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifdef COMPILED_ON_LITTLE_ENDIAN

uint16_t htovm_16(uint16_t s);
uint32_t htovm_32(uint32_t l);
uint64_t htovm_64(uint64_t ll);

#else
typedef union
{
    uint8_t c[4];
    uint16_t s[2];
    uint32_t l;
} U4;

typedef union
{
    uint8_t c[8];
    uint8_t c2[2][4];
    uint16_t s[4];
    uint32_t l[2];
    uint64_t ll;
} U8;

typedef union
{
    uint8_t c[2];
    uint16_t s;
} U2;

uint16_t htovm_16(uint16_t s);
uint32_t htovm_32(uint32_t l);
uint64_t htovm_64(uint64_t ll);

#endif

#ifdef __cplusplus
}
#endif


#endif
