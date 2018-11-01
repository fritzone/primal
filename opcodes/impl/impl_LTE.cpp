#include <LTE.h>
#include <vm.h>

#include <iostream>

bool primal::impl_LTE(primal::vm* v)
{
    primal::valued* first = v->fetch();
    primal::valued* second = v->fetch();

    v->flag() = *first <= *second;

    return true;
}

