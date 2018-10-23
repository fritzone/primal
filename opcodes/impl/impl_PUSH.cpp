#include <PUSH.h>
#include <vm.h>

#include <iostream>

bool primate::impl_PUSH(primate::vm* v)
{
    std::cout << "PUSH" << std::endl;
    return false;
}

