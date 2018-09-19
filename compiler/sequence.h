#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "lexer.h"

#include <memory>
#include <string>
#include <vector>

class source;
class ast;
class compiler;

/** A sequence represents a keyword and its attached expressions */
class sequence
{
public:
    virtual ~sequence() = default;

    /* Creates a new sequence from the given source */
    explicit sequence(source& src) : m_src(src) {}

    /* This method will do some preliminary preparations on the token vector */
    virtual bool prepare(std::vector<token>& tokens) = 0;

    /* This compiles the expression that remained after prepare. Overrides of this method
    should call this first. This leaves the result in R0 */
    virtual bool compile(compiler* c);

    /* Returns the root of the abstract syntax tree */
    std::shared_ptr<ast>& root();

public:

    /* Factory method to create a sequence for the given keyword */
    static std::shared_ptr<sequence> create(std::vector<token> &tokens, source& src);

private:

    virtual void traverse_ast(uint8_t level, const std::shared_ptr<ast>& croot, compiler* c);

protected:

    source& m_src;
    std::shared_ptr<ast> m_root;
};

#endif

