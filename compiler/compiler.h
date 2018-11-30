#ifndef COMPILER_H
#define COMPILER_H

#include <memory>
#include <vector>
#include <map>

#include "source.h"

namespace primal
{
    class generate;
    class fun;
    class variable;

    /* The wrapper of the compilers' notion */
    class compiler
    {
    public:

        /* this needs to be called before you want to compile */
        static std::shared_ptr<compiler> create();

    public:

        compiler() = default;
        virtual ~compiler();

        /* Will compile the given instructions, each of them must be newline separated */
        bool compile(const std::string& s);

        /* Will retrieve the bytecode of the latest compilation. */
        std::vector<uint8_t> bytecode() const;

        /* Will yield the code generator assigned to this compiler */
        std::shared_ptr<generate> generator();

        /* Will return the source on which this compiler operates */
        source& get_source();

        /* will return the number assigned to the next variable */
        int next_varcount(fun* holder, const std::string &name);
        int last_varcount(fun* holder);
        bool has_variable(const std::string & name);
        std::shared_ptr<variable> get_variable(const std::string & name);
        std::shared_ptr<variable> create_variable(const std::string& name);

        /* sets the current working frame */
        void set_frame(fun* f);
        fun* frame() const { return m_current_frame; }

        std::string preprocess(const std::string& s);

    private:

        source m_src;
        std::map<fun*, int> m_varcounters;
        std::map<fun*, std::map<std::string, std::shared_ptr<variable>>> variables;
        fun* m_current_frame = nullptr;
    };
}

#endif
