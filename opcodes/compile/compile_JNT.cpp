#include <JNT.h>
#include <token.h>

#include <iostream>
#include <vector>

std::vector<uint8_t> compile_JNT(std::vector<token>&)
{
    std::cout << "compile: JNT" << std::endl;
    return std::vector<uint8_t>();
}

