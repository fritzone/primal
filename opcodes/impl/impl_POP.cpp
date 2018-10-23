#include <POP.h>
#include <vm.h>

#include <iostream>

bool primate::impl_POP(primate::vm* v)
{
    std::cout << "POP" << std::endl;
    return false;
}

