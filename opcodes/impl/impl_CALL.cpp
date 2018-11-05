#include <CALL.h>
#include <vm.h>

#include <iostream>

bool primal::impl_CALL(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    // now push the current IP
    v->push( v->ip() );
    // and now just go to the address where the dest points
    v->ip() = dest->value();
    return true;
}

