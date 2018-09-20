#include <JMP.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_JMP(std::vector<token>&)
{
    std::cout << "compile: JMP" << std::endl;
    return std::vector<uint8_t>();
}

