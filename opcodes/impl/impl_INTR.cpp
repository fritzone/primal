#include <INTR.h>
#include <vm.h>

bool primal::impl_INTR(primal::vm* v)
{
    v->debug(opcodes::INTR(), OpcodeDebugState::VM_DEBUG_BEFORE);
    auto* intnr = v->fetch();
    v->interrupt(intnr->value());
    v->debug(opcodes::INTR(), OpcodeDebugState::VM_DEBUG_AFTER);
    return true;
}

