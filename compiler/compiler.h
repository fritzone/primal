#ifndef COMPILER_H
#define COMPILER_H

#include <memory>
#include <vector>
#include "source.h"

class generate;

class compiler
{
public:

    static std::shared_ptr<compiler> initalize();

    compiler() = default;

    bool compile(const std::string& s);
    std::vector<uint8_t> bytecode() const;
    std::shared_ptr<generate> gen_code();
    source& get_source();

private:

    source m_src;
};

#endif

