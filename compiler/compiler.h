#ifndef COMPILER_H
#define COMPILER_H

#include <memory>
#include <vector>
#include <map>

#include "source.h"
#include "numeric_decl.h"

namespace primal
{
    class generate;
    class variable;
    class fun;

    /**
         * @brief A plain, serializable structure to hold a function's summary.
         *
         * This can be written directly into the bytecode to form a symbol table.
         */
    struct summary_pod {
        // The name of the function. max 255
        char name[255] = {0};

        // The address of the function within the bytecode.
        word_t address;

        // The number of parameters this function takes.
        uint8_t parameter_count {0};

        // A flag indicating if the function is a C++ extern call.
        uint8_t is_extern {0};

        // The return type of the function, encoded as entity_type.
        uint8_t return_type {0};

        // The types of each parameter, encoded as entity_type.
        uint8_t parameter_types[255] = {0};
    };

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

        std::map<std::string, std::shared_ptr<fun> > print_function_summary();
        std::vector<summary_pod> get_function_summaries() const;
        void set_interface_header_path(const std::string& path, const std::string& script_name);
    private:
        void generate_variable_interface() const;

        source m_src;
        std::map<fun*, int> m_varcounters;
        std::map<fun*, std::map<std::string, std::shared_ptr<variable>>> variables;
        fun* m_current_frame = nullptr;
        std::string m_interface_header_path;
        std::string m_script_name;

    };
}

#endif
