#include <JT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JT(primal::vm* v)
{
    auto* loc = v->fetch();
    if(v->flag()) v->ip() = loc->value();
    v->flag()= false;
    return true;
}

