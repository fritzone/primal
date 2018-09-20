#include <ADD.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_ADD(std::vector<token>&)
{
    std::cout << "compile: ADD" << std::endl;
    return std::vector<uint8_t>();
}

