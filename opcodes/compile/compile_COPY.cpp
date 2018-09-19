#include <COPY.h>
#include <token.h>

#include <iostream>
#include <vector>

bool compile_COPY(std::vector<token>&)
{
    std::cout << "compile: COPY" << std::endl;
    return false;
}

