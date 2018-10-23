#include <MOV.h>
#include <vm.h>
#include <types.h>

using namespace primate;

bool primate::impl_MOV(primate::vm* v)
{
    auto* dest = v->fetch();
    auto* src  = v->fetch();
    dest->set_value(src->value());
    v->flag() = (dest->value() != 0);
    return true;
}

