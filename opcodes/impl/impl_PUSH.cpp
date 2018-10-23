#include <PUSH.h>
#include <vm.h>

#include <iostream>

bool primal::impl_PUSH(primal::vm* v)
{
    std::cout << "PUSH" << std::endl;
    return false;
}

