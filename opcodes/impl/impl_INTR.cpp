#include <INTR.h>
#include <vm.h>

bool primal::impl_INTR(primal::vm* v)
{
    auto* intnr = v->fetch();
    v->interrupt(intnr->value());
    return true;
}

