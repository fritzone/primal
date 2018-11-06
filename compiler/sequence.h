#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "lexer.h"

#include <memory>
#include <string>
#include <vector>

namespace primal
{
    class source;
    class ast;
    class compiler;
    class function;

    /* A sequence represents a keyword and its attached expressions */
    class sequence
    {
    public:

        enum class prepared_type
        {
            PT_NORMAL           = 1,
            PT_PRECOMPILED      = 2,
            PT_CONSUMED         = 3,
            PT_FUNCTION_CALL    = 4,
            PT_FUNCTION_DECL    = 5,
            PT_INVALID          = 255
        };

        virtual ~sequence() = default;

        /* Creates a new sequence from the given source */
        explicit sequence(source& src) : m_src(src) {}

        /* This method will do some preliminary preparations on the token vector */
        virtual prepared_type prepare(std::vector<token>& tokens) = 0;

        /* This compiles the expression that remained after prepare. Overrides of this method
        should call this first. This leaves the result in $r0 */
        virtual bool compile(compiler* c);

        /* Returns the root of the abstract syntax tree */
        std::shared_ptr<ast>& root();

    public:

        /* Factory method to create a sequence for the given keyword */
        static std::shared_ptr<sequence> create(std::vector<token> &tokens, source& src);

    protected:

        virtual void traverse_ast(uint8_t level, const std::shared_ptr<ast>& croot, compiler* c);

    protected:

        source& m_src;
        std::shared_ptr<ast> m_root;
    };

}


#endif

