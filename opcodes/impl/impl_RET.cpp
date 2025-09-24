#include <RET.h>
#include <vm.h>

#include <iostream>

bool primal::impl_RET(primal::vm* v)
{
    v->debug(opcodes::RET(), OpcodeDebugState::VM_DEBUG_BEFORE);
    word_t dest = v->pop();
    v->ip() = dest;
    v->debug(opcodes::RET(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

