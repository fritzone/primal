#include <DIV.h>
#include <vm.h>

#include <iostream>

bool impl_DIV(vm* v)
{
    valued* dest = v->fetch();
    valued* src  = v->fetch();

    if(src->value() == 0)
    {
        v->panic();
    }

    *dest /= *src;

    return true;
}

