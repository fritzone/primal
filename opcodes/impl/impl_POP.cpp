#include <POP.h>
#include <vm.h>

#include <iostream>

bool impl_POP(vm* v)
{
    std::cout << "POP" << std::endl;
    return false;
}

