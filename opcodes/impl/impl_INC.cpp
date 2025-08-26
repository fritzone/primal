#include <INC.h>
#include <vm.h>


bool primal::impl_INC(vm* v)
{
    valued* dest = v->fetch();
    dest->set_value(dest->value() + 1);
    return true;
}
