#include <LT.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_LT(std::vector<token>&)
{
    std::cout << "compile: LT" << std::endl;
    return std::vector<uint8_t>();
}

