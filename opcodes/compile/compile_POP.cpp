#include <POP.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_POP(std::vector<token>&)
{
    std::cout << "compile: POP" << std::endl;
    return std::vector<uint8_t>();
}

