#include <XOR.h>
#include <vm.h>

#include <iostream>

bool impl_XOR(vm* v)
{
    valued* dest = v->fetch();
    valued* src  = v->fetch();

    *dest ^= *src;

    return true;
}

