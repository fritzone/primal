#include <NOT.h>
#include <vm.h>

#include <iostream>

bool primate::impl_NOT(primate::vm* v)
{
    primate::valued* dest = v->fetch();
    dest->set_value(!dest->value());
    v->flag() = (dest->value() != 0);
    return true;
}

