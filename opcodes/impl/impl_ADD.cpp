#include <ADD.h>
#include <vm.h>

#include <iostream>

bool primal::impl_ADD(primal::vm* v)
{
    v->debug(opcodes::ADD(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest += *src;

    v->set_flag(dest->value() != 0);

    v->debug(opcodes::ADD(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

