#include <JNT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JNT(primal::vm* v)
{
    v->debug(opcodes::JNT(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* loc = v->fetch();
    if(!v->flag()) return v->jump(loc->value());
    v->set_flag( 0 );
    v->debug(opcodes::JNT(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

