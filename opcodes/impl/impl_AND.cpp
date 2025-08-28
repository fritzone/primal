#include <AND.h>
#include <vm.h>

#include <iostream>

bool primal::impl_AND(primal::vm* v)
{
    v->debug(opcodes::AND(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest &= *src;
    v->set_flag(dest->value() != 0);
    v->debug(opcodes::AND(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

