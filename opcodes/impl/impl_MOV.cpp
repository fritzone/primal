#include <MOV.h>
#include <vm.h>

#include <iostream>

using namespace primal;

bool primal::impl_MOV(primal::vm* v)
{
    auto* dest = v->fetch();
    auto* src = v->fetch();
#ifdef _LOWLEVEL_EXEC_DEBUG
    std::cout << "mov: dest=" << dest->debug() << " src=" << src->debug() << std::endl;
#endif
    dest->set_value(src->value());
    v->set_flag(dest->value() != 0);
    return true;
}

