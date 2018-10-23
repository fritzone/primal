#ifndef COMPILER_H
#define COMPILER_H

#include <memory>
#include <vector>
#include "source.h"

namespace primal
{
    class generate;

    /* The wrapper of the compilers' notion */
    class compiler
    {
    public:

        /* this needs to be called before you want to compile */
        static std::shared_ptr<compiler> create();

    public:

        compiler() = default;
        virtual ~compiler();

        /* Will compile the given instructions, each of the mmust be newline separated */
        bool compile(const std::string& s);

        /* Will retrieve the bytecode of the latest compilation. */
        std::vector<uint8_t> bytecode() const;

        /* Will yield the code generator assigned to this compiler */
        std::shared_ptr<generate> generator();

        /* Will return the source on which this compiler operates */
        source& get_source();

    private:

        source m_src;
    };

}

#endif

