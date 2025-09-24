#include <JMP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JMP(primal::vm* v)
{
    v->debug(opcodes::JMP(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* loc = v->fetch();
    return v->jump(loc->value());
}

