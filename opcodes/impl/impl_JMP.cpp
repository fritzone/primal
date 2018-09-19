#include <JMP.h>
#include <vm.h>

#include <iostream>

bool impl_JMP(vm* v)
{
    std::cout << "JMP" << std::endl;
    return false;
}

