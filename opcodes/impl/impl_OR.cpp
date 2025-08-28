#include <OR.h>
#include <vm.h>

#include <iostream>

bool primal::impl_OR(primal::vm* v)
{
    v->debug(opcodes::OR(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest |= *src;
    v->set_flag(*dest != 0);
    v->debug(opcodes::OR(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

