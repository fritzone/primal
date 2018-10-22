#include <OR.h>
#include <vm.h>

#include <iostream>

bool impl_OR(vm* v)
{
    valued* dest = v->fetch();
    valued* src  = v->fetch();

    *dest |= *src;
    v->flag() = (dest->value() != 0);
    return true;
}

