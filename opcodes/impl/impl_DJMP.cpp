#include <DJMP.h>
#include <vm.h>

#include <iostream>

bool impl_DJMP(vm* v)
{
    std::cout << "DJMP" << std::endl;
    return false;
}

