#include <MOV.h>
#include <vm.h>
#include <types.h>

bool impl_MOV(vm* v)
{
    auto* dest = v->fetch();
    auto* src  = v->fetch();
    dest->set_value(src->value());
    return true;
}

