#include <NOT.h>
#include <vm.h>

#include <iostream>

bool primal::impl_NOT(primal::vm* v)
{
    primal::valued* dest = v->fetch();
    dest->set_value(!dest->value());
    v->set_flag(dest->value() != 0);
    return true;
}

