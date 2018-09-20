#include <MOD.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_MOD(std::vector<token>&)
{
    std::cout << "compile: MOD" << std::endl;
    return std::vector<uint8_t>();
}

