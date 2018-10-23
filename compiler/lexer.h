#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

#include "token.h"

namespace primal
{
    /* This class is the lexer ofthe compiler */
    class lexer
    {
    public:

        /* Constructor */
        explicit lexer(std::string s) : m_sequence(std::move(s)) {}

        /* Will create a vector of tokens */
        std::vector<token> tokenize();

    private:

        // the code sequence on which we work
        std::string m_sequence;
    };

}

#endif // LEXER_H

