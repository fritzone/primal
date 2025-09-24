#include "byte_order.h"

#include <stdint.h>

/* all this gets converted to little endian */

#ifndef COMPILED_ON_LITTLE_ENDIAN

uint32_t htovm_32(uint32_t l)
{
    U4 mask,res;
    unsigned char* p = (unsigned char*)&l;
    mask.l = 0x03020100;
#ifdef _WINDOWS
#pragma warning(suppress: 6386)
#endif
    res.c[(uint8_t)(mask.c[0])] = (uint8_t)p[0];
    res.c[(uint8_t)(mask.c[1])] = (uint8_t)p[1];
    res.c[(uint8_t)(mask.c[2])] = (uint8_t)p[2];
    res.c[(uint8_t)(mask.c[3])] = (uint8_t)p[3];
    return res.l;
}

uint64_t htovm_64(uint64_t ll)
{
    U2 mask1;
    U4 mask2;
    U8 res;

    unsigned char* p = (unsigned char*)&ll;
    mask1.s = 0x0100;
    mask2.l = 0x03020100;
#ifdef _WINDOWS
#pragma warning(suppress: 6386)
#endif
    res.c2[mask1.c[0]][mask2.c[0]] = p[0];
    res.c2[mask1.c[0]][mask2.c[1]] = p[1];
    res.c2[mask1.c[0]][mask2.c[2]] = p[2];
    res.c2[mask1.c[0]][mask2.c[3]] = p[3];
#ifdef _WINDOWS
#pragma warning(suppress: 6386)
#endif
    res.c2[mask1.c[1]][mask2.c[0]] = p[4];
    res.c2[mask1.c[1]][mask2.c[1]] = p[5];
    res.c2[mask1.c[1]][mask2.c[2]] = p[6];
    res.c2[mask1.c[1]][mask2.c[3]] = p[7];
    return res.ll;
}

uint16_t htovm_16(uint16_t s)
{
    U2 mask,res;
    unsigned char* p = (unsigned char*)&s;
    mask.s = 0x0100;
#ifdef _WINDOWS
#pragma warning(suppress: 6386)
#endif
    res.c[mask.c[0]] = p[0];
    res.c[mask.c[1]] = p[1];
    return res.s;
}
#else
uint16_t htovm_16(uint16_t s) {return s; }
uint32_t htovm_32(uint32_t l) {return l; }
uint64_t htovm_64(uint64_t ll){return ll;}
#endif
