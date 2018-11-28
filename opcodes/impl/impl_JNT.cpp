#include <JNT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JNT(primal::vm* v)
{
    auto* loc = v->fetch();
    if(!v->flag()) return v->jump(loc->value());
    v->set_flag( 0 );
    return true;
}

