#include <PUSH.h>
#include <vm.h>

#include <iostream>

bool impl_PUSH(vm* v)
{
    std::cout << "PUSH" << std::endl;
    return false;
}

