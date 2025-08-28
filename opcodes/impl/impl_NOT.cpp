#include <NOT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_NOT(primal::vm* v)
{
    v->debug(opcodes::NOT(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    dest->set_value(!dest->value());
    v->set_flag(dest->value() != 0);
    v->debug(opcodes::NOT(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

