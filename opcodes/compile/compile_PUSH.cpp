#include <PUSH.h>
#include <token.h>

#include <iostream>
#include <vector>

bool compile_PUSH(std::vector<token>&)
{
    std::cout << "compile: PUSH" << std::endl;
    return false;
}

