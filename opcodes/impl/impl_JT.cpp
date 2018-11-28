#include <JT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JT(primal::vm* v)
{
    auto* loc = v->fetch();
    if(v->flag()) return v->jump(loc->value());
    v->set_flag( 0 );
    return true;
}

