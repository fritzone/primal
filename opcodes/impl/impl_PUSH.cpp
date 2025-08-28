#include <PUSH.h>
#include <vm.h>

#include <iostream>

bool primal::impl_PUSH(primal::vm* v)
{
    v->debug(opcodes::PUSH(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* t = v->fetch();
    bool result = v->push(t);
    v->debug(opcodes::PUSH(), OpcodeDebugState::VM_DEBUG_AFTER);
    return result;
}

