#include <DJMP.h>
#include <vm.h>

#include <iostream>

bool primate::impl_DJMP(primate::vm* v)
{
    std::cout << "DJMP" << std::endl;
    return false;
}

