#include <DJT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJT(primal::vm* v)
{
    auto* delta = v->fetch();
    if(v->flag()) v->ip() += delta->value();
    v->flag()= false;
    return true;
}

