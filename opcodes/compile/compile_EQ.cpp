#include <EQ.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_EQ(std::vector<token>&)
{
    std::cout << "compile: EQ" << std::endl;
    return std::vector<uint8_t>();
}

