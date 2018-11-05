#include <RET.h>
#include <vm.h>

#include <iostream>

bool primal::impl_RET(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    return true;
}

