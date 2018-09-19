#include <JMP.h>
#include <token.h>

#include <iostream>
#include <vector>

bool compile_JMP(std::vector<token>&)
{
    std::cout << "compile: JMP" << std::endl;
    return false;
}

