#include <POP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_POP(primal::vm* v)
{
    v->debug(opcodes::POP(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* t = v->fetch();
    word_t p = v->pop();
    t->set_value(p);
    v->debug(opcodes::POP(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

