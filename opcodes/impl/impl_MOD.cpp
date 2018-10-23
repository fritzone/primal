#include <MOD.h>
#include <vm.h>

#include <iostream>

bool primal::impl_MOD(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest %= *src;
    v->flag() = (dest->value() != 0);
    return true;
}

