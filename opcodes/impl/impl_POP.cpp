#include <POP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_POP(primal::vm* v)
{
    auto* t = v->fetch();
    word_t p = v->pop();
    t->set_value(p);
    return true;
}

