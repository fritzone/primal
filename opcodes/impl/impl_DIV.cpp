#include <DIV.h>
#include <vm.h>

#include <iostream>

bool primal::impl_DIV(primal::vm* v)
{
    v->debug(opcodes::DIV(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    primal::valued* src  = v->fetch();

    if(src->value() == 0)
    {
        v->panic("Division by 0");
    }

    *dest /= *src;
    v->set_flag(dest->value() != 0);
    v->debug(opcodes::DIV(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

