#include <DJNT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJNT(primal::vm* v)
{
    v->debug(opcodes::DJNT(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* delta = v->fetch();
    if(!v->flag()) v->ip() += delta->value();
    v->set_flag(0);
    v->debug(opcodes::DJNT(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

