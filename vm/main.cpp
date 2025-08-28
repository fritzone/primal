#include "util.h"

#include "vm.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

[[noreturn]] static void usage()
{
    std::cout << "Primal VM" << std::endl;
    std::cout << "Usage: primv <app.pric>" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    util::InputParser input(argc, argv);
    if(input.cmdOptionExists("-h") || argc <= 1)
    {
        usage();
    }

    std::vector<uint8_t> vec;

    std::ifstream file(argv[1], std::ios::binary);

    file.unsetf(std::ios::skipws);

    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    vec.reserve(fileSize);

    // read the data:
    vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

    auto vm = primal::vm::create();

    if (input.cmdOptionExists("-d") || input.cmdOptionExists("--debug"))
    {
        vm->set_debug(true);
        std::cout << "--- VM DEBUG ENABLED ---" << std::endl;
    }

    vm->run(vec);
}
