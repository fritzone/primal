#include <DIV.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DIV(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    if(src->value() == 0)
    {
        v->panic();
    }

    *dest /= *src;
    v->flag() = (dest->value() != 0);
    return true;
}

