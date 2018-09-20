#include <OR.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_OR(std::vector<token>&)
{
    std::cout << "compile: OR" << std::endl;
    return std::vector<uint8_t>();
}

