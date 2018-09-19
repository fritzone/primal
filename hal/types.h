#ifndef TYPES_H
#define TYPES_H

#include <cstdint>

#include "byte_order.h"

template <class T> T htovm(T t);

// this is the default type for the numbers in the VM
#if TARGET_ARCH == 32
    using numeric_t = ::int32_t;
    template <>
    inline numeric_t htovm<numeric_t>(numeric_t l) {return static_cast<numeric_t>(htovm_32(static_cast<uint32_t>(l))); }

#elif TARGET_ARCH == 64
    using numeric_t = ::int64_t;
    template <>
    inline numeric_t htovm<numeric_t>(numeric_t l) {return static_cast<numeric_t>(htovm_64(static_cast<uint64_t>(l))); }
#else
    static_assert(0, "This seems to be a not properly configured compilation, please run cmake")
#endif

enum class type_destination : uint8_t
{
    TYPE_MOD_IMM = 0x00,   // what follows is a 32 bit integer value
    TYPE_MOD_REG = 0x01,   // what follows is the index of a register, 1 byte
    TYPE_MOD_VAR = 0x02,   // what follows is the index of a variable from the variables stack
    TYPE_MOD_MEM_IMM = 0x03,   // what follows is the address of a memory cell, byte, immediate value
    TYPE_MOD_STR_IMM = 0x04,   // what follows is the address of a string cell, immediate value
};


#endif
