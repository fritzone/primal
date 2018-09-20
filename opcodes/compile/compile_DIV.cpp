#include <DIV.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_DIV(std::vector<token>&)
{
    std::cout << "compile: DIV" << std::endl;
    return std::vector<uint8_t>();
}

