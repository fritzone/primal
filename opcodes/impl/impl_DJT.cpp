#include <DJT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJT(primal::vm* v)
{
    v->debug(opcodes::DJT(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* delta = v->fetch();
    if(v->flag()) v->ip() += delta->value();
    v->set_flag(0);
    v->debug(opcodes::DJT(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

