#ifndef OPTIONS_H
#define OPTIONS_H

#include <sstream>

namespace primal
{
    /*Class representing some options*/
    class options
    {
    public:

        static options& instance();

        bool generate_assembly() const;

        std::stringstream& asm_stream() { return m_asm_stream; }

    private:

        options() = default;

        // whether the compilation phase will also show the generated assembly
        bool m_generate_assembly = true;

        std::stringstream m_asm_stream;
    };

}


#endif
