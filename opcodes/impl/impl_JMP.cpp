#include <JMP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JMP(primal::vm* v)
{
    auto* loc = v->fetch();
    return v->jump(loc->value());
}

