#include <DJMP.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_DJMP(std::vector<token>&)
{
    std::cout << "compile: DJMP" << std::endl;
    return std::vector<uint8_t>();
}

