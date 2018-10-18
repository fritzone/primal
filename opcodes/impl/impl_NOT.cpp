#include <NOT.h>
#include <vm.h>

#include <iostream>

bool impl_NOT(vm* v)
{
    valued* dest = v->fetch();
    dest->set_value(!dest->value());

    return true;
}

