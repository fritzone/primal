#include <EQ.h>
#include <vm.h>

#include <iostream>

bool primate::impl_EQ(primate::vm* v)
{
    std::cout << "EQ" << std::endl;
    return false;
}

