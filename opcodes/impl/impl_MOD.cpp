#include <MOD.h>
#include <vm.h>

#include <iostream>

bool primal::impl_MOD(primal::vm* v)
{
    v->debug(opcodes::MOD(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    *dest %= *src;
    v->set_flag(*dest != 0);
    v->debug(opcodes::MOD(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

