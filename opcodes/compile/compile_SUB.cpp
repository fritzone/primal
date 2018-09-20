#include <SUB.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_SUB(std::vector<token>&)
{
    std::cout << "compile: SUB" << std::endl;
    return std::vector<uint8_t>();
}

