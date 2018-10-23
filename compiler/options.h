#ifndef OPTIONS_H
#define OPTIONS_H

namespace primal
{
    /*Class representing some options*/
    class options
    {
    public:

        static options& instance();

        bool generate_assembly() const;

    private:

        options() = default;

        // whether the compilation phase will also show the generated assembly
        bool m_generate_assembly = true;
    };

}


#endif
