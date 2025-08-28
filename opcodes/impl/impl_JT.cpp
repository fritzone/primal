#include <JT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JT(primal::vm* v)
{
    v->debug(opcodes::JT(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* loc = v->fetch();
    if(v->flag()) return v->jump(loc->value());
    v->set_flag( 0 );
    v->debug(opcodes::JT(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

