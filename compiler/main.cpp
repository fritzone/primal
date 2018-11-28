#include "compiler.h"
#include "util.h"

#include <iostream>
#include <string>
#include <fstream>
#include <streambuf>
#include <iomanip>

static void usage()
{
    std::cout << "Primal compiler" << std::endl;
    std::cout << "Usage: primc -i <input.prim> -o <output.pric>" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    util::InputParser input(argc, argv);
    if(input.cmdOptionExists("-h") || argc == 1)
    {
        usage();
    }

    std::string infile = input.getCmdOption("-i");
    std::string outfile = input.getCmdOption("-o");

    if(infile.empty() || outfile.empty())
    {
        usage();
    }


    std::ifstream t(infile);
    std::string app((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());

    auto c = primal::compiler::create();
    c->compile(app);

    std::vector<uint8_t> compiled_app = c->bytecode();

    std::ofstream outf(outfile, std::ios::out | std::ios::binary);
    outf.write(reinterpret_cast<char*>(&compiled_app[0]), static_cast<std::streamsize>(compiled_app.size() * sizeof(uint8_t)));
    outf.close();

}
