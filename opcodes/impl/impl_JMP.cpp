#include <JMP.h>
#include <vm.h>

#include <iostream>

bool primate::impl_JMP(primate::vm* v)
{
    std::cout << "JMP" << std::endl;
    return false;
}

