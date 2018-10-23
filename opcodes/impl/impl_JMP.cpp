#include <JMP.h>
#include <vm.h>

#include <iostream>

bool primal::impl_JMP(primal::vm* v)
{
    std::cout << "JMP" << std::endl;
    return false;
}

