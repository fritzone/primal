#include <COPY.h>
#include <vm.h>

#include <iostream>

bool primal::impl_COPY(primal::vm* v)
{
    v->debug(opcodes::COPY(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* dest = v->fetch();
    auto* src  = v->fetch();
    auto* cnt = v->fetch();

    return v->copy(dest->value(), src->value(), cnt->value());
}

