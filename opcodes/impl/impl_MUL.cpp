#include <MUL.h>
#include <vm.h>

#include <iostream>

bool primal::impl_MUL(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest *= *src;
    v->set_flag(dest->value() != 0);
    return true;
}

