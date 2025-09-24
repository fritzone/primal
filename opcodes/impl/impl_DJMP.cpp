#include <DJMP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DJMP(primal::vm* v)
{
    v->debug(opcodes::DJMP(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* delta = v->fetch();
    v->ip() += delta->value();
    v->debug(opcodes::DJMP(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

