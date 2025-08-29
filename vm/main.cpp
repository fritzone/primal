#include "util.h"

#include "vm.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>

[[noreturn]] static void usage()
{
    std::cout << "Primal VM" << std::endl;
    std::cout << "Usage: primv <app.pric> [-d|--debug] [--speed <Hz>]" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    util::InputParser input(argc, argv);
    if(input.cmdOptionExists("-h") || argc <= 1)
    {
        usage();
    }

    // MODIFIED: Robustly find the filename, ignoring flags.
    std::string filename;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        // If the argument is a known flag, skip it.
        if (arg == "-h" || arg == "-d" || arg == "--debug" || arg == "--speed") {
            // If the flag takes a value (like --speed), skip the next argument as well.
            if (arg == "--speed") {
                i++;
            }
            continue;
        }
        // The first argument that is not a flag is our filename.
        if (filename.empty()) {
            filename = arg;
        } else {
            // We have already found a filename, so this is an extra, unknown argument.
            std::cerr << "Error: Unexpected argument '" << arg << "'" << std::endl;
            usage();
        }
    }

    if (filename.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        usage();
    }


    std::vector<uint8_t> vec;
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error: Cannot open input file '" << filename << "'" << std::endl;
        return 1;
    }

    file.unsetf(std::ios::skipws);
    std::streampos fileSize;
    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    vec.reserve(fileSize);
    vec.insert(vec.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

    auto vm = primal::vm::create();

    if (input.cmdOptionExists("-d") || input.cmdOptionExists("--debug")) {
        vm->set_debug(true);
        std::cout << "--- VM DEBUG ENABLED ---" << std::endl;
    }

    // MODIFIED: Check for the --speed flag and set the VM's clock speed.
    const std::string& speed_str = input.getCmdOption("--speed");
    if (!speed_str.empty()) {
        try {
            uint64_t speed_hz = std::stoull(speed_str);
            if (speed_hz > 0) {
                vm->set_speed(speed_hz);
                std::cout << "--- VM SPEED CAPPED AT " << speed_hz << " Hz ---" << std::endl;
            }
        } catch (const std::exception&) {
            std::cerr << "Error: Invalid value for --speed argument." << std::endl;
            usage();
        }
    }

    vm->run(vec);
}
