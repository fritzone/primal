#include <PUSH.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_PUSH(std::vector<token>&)
{
    std::cout << "compile: PUSH" << std::endl;
    return std::vector<uint8_t>();
}

