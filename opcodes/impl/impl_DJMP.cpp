#include <DJMP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJMP(primal::vm* v)
{
    auto* delta = v->fetch();
    v->ip() += delta->value();
    return true;
}

