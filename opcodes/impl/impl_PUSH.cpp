#include <PUSH.h>
#include <vm.h>

#include <iostream>

bool primal::impl_PUSH(primal::vm* v)
{
    auto* t = v->fetch();
    return v->push(t);
}

