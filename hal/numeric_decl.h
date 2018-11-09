#ifndef NUMERIC_DECL_H
#define NUMERIC_DECL_H

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

// the size of it, just to not to have to use sizeof everywhere in the code
const int num_t_size = static_cast<numeric_t>(sizeof(numeric_t));

#endif
