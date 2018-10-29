#include <GT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_GT(primal::vm* v)
{
    primal::valued* first = v->fetch();
    primal::valued* second = v->fetch();

    v->flag() = first->value() > second->value();

    return true;
}

