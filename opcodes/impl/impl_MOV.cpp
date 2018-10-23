#include <MOV.h>
#include <vm.h>
#include <types.h>

using namespace primal;

bool primal::impl_MOV(primal::vm* v)
{
    auto* dest = v->fetch();
    auto* src  = v->fetch();
    dest->set_value(src->value());
    v->flag() = (dest->value() != 0);
    return true;
}

