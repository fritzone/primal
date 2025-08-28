#include <NEQ.h>
#include <vm.h>

#include <iostream>

bool primal::impl_NEQ(primal::vm* v)
{
    v->debug(opcodes::NEQ(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* first = v->fetch();
    primal::valued* second = v->fetch();

    v->set_flag(*first != *second);

    v->debug(opcodes::NEQ(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

