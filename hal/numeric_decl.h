#ifndef NUMERIC_DECL_H
#define NUMERIC_DECL_H

#include "byte_order.h"

template <class T> T htovm(T t);

#ifndef TARGET_ARCH
#define TARGET_ARCH 64
#endif

// this is the default type for the numbers in the VM
#if TARGET_ARCH == 32
    using word_t = ::int32_t;
    template <>
    inline word_t htovm<word_t>(word_t l) {return static_cast<word_t>(htovm_32(static_cast<uint32_t>(l))); }
#elif TARGET_ARCH == 64
    using word_t = ::int64_t;
    template <>
    inline word_t htovm<word_t>(word_t l) {return static_cast<word_t>(htovm_64(static_cast<uint64_t>(l))); }
#else
    static_assert(0, "This seems to be a not properly configured compilation, please run cmake")
#endif

// the size of it, just to not to have to use sizeof everywhere in the code
constexpr word_t word_size = static_cast<word_t>(sizeof(word_t));

#endif
