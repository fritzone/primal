#ifndef OPTIONS_H
#define OPTIONS_H

#include <sstream>
#include <iostream>

namespace primal
{
    /*Class representing some options*/
    class options
    {
    public:

        static options& instance();

        bool generate_assembly() const;
        void generate_assembly(bool);

        auto& asm_stream() { return std::cout; }

    private:

        options() = default;

        // whether the compilation phase will also show the generated assembly
        bool m_generate_assembly = false;

//        auto m_asm_stream = std::cout;
    };

}


#endif
