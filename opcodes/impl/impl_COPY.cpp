#include <COPY.h>
#include <vm.h>

#include <iostream>

bool impl_COPY(vm* v)
{
    std::cout << "COPY" << std::endl;
    return false;
}

