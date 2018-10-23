#include <ADD.h>
#include <vm.h>

#include <iostream>

bool primate::impl_ADD(primate::vm* v)
{
    primate::valued* dest = v->fetch();
    primate::valued* src  = v->fetch();

    *dest += *src;

    v->flag() = (dest->value() != 0);

    return true;
}

