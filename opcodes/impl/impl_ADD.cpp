#include <ADD.h>
#include <vm.h>

#include <iostream>

bool impl_ADD(vm* v)
{
    valued* dest = v->fetch();
    valued* src  = v->fetch();

    *dest += *src;

    return true;
}

