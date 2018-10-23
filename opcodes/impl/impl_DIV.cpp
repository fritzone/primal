#include <DIV.h>
#include <vm.h>

#include <iostream>

bool primate::impl_DIV(primate::vm* v)
{
    primate::valued* dest = v->fetch();
    primate::valued* src  = v->fetch();

    if(src->value() == 0)
    {
        v->panic();
    }

    *dest /= *src;
    v->flag() = (dest->value() != 0);
    return true;
}

