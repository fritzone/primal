#include <MUL.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_MUL(std::vector<token>&)
{
    std::cout << "compile: MUL" << std::endl;
    return std::vector<uint8_t>();
}

