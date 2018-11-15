#include <DJNT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJNT(primal::vm* v)
{
    auto* delta = v->fetch();
    if(!v->flag()) v->ip() += delta->value();
    v->flag()= 0;
    return true;
}

