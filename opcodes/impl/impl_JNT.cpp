#include <JNT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JNT(primal::vm* v)
{
    auto* loc = v->fetch();
    if(!v->flag()) v->ip() = loc->value();
    v->flag()= false;
    return true;
}

