#include <MOV.h>
#include <vm.h>

using namespace primal;

bool primal::impl_MOV(primal::vm* v)
{
    auto* dest = v->fetch();
    auto* src = v->fetch();

    word_t vv = src->value();

    dest->set_value(vv);
    v->set_flag((dest->value() != 0));
    return true;
}

