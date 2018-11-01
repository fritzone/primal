#include <OR.h>
#include <vm.h>

#include <iostream>

bool primal::impl_OR(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest |= *src;
    v->flag() = (*dest != 0);
    return true;
}

