#include <RET.h>
#include <vm.h>

#include <iostream>

bool primal::impl_RET(primal::vm* v)
{
    word_t dest = v->pop();
    v->ip() = dest;
    return true;
}

