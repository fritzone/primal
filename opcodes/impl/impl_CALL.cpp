#include <CALL.h>
#include <vm.h>

#include <iostream>
#include <sstream>

bool primal::impl_CALL(primal::vm* v)
{
    v->debug(opcodes::CALL(), OpcodeDebugState::VM_DEBUG_BEFORE);
    primal::valued* dest = v->fetch();
    // now push the current IP
    v->push( v->ip() );
    // and now just go to the address where the dest points
    bool result = v->jump(dest->value());
    v->debug(opcodes::CALL(), OpcodeDebugState::VM_DEBUG_AFTER);
    return result;
}
