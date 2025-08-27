#include "kw_let.h"
#include "variable.h"
#include "generate.h"
#include "opcodes.h"
#include "registers.h"
#include "compiler.h"
#include "exceptions.h"

#include <parser.h>

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_let::prepare(std::vector<token> &tokens)
{
    std::vector<token> token_copy = tokens;
    if(tokens.empty())
    {
        return sequence::prepared_type::PT_INVALID;
    }
    // fetching the name of the token, see if it's a valid identifier or not
    m_name = tokens[0].data();
    tokens.erase(tokens.begin());
    if(!variable::has_variable(m_name))
    {
        throw primal::syntax_error("not found a variable declaration: " + m_name);
    }

    // erasing the equality sign, check for error
    if(tokens.empty())
    {
        throw primal::syntax_error("incomplete LET command");
    }

    // now deal with the following scenarios:
    // 1. the first token is [. This means indexed access
    if(tokens[0].data() != "[")
    {
        tokens.erase(tokens.begin());
        if(tokens.empty())
        {
            return sequence::prepared_type::PT_INVALID;
        }
    }
    else
    {
        // remove the opening bracket
        tokens.erase(tokens.begin());
        if(tokens.empty())
        {
            return sequence::prepared_type::PT_INVALID;
        }

        // fetch the value that is being placed in there
        while(!tokens.empty() && tokens[0].data() != "]")
        {
            m_index_expression += tokens[0].data();
            tokens.erase(tokens.begin());
        }

        if(!m_index_expression.empty())
        {
            // creating a new vector of tokens for the index
            m_indexed = true;
            lexer l(m_index_expression);
            m_index_tokens = l.tokenize();

            m_index_seq = sequence::create(m_index_tokens, source::get());
            // create the RPN for the expression
            std::vector<token> output = parser::shuntyard(m_index_tokens);

            // build the abstract syntax tree for the result of the shuntyard
            ast::build_ast(output, m_index_seq->root());

        }
    }

    // now we should go through the tokens one by one and see which one of them is a function call

    return sequence::prepared_type::PT_NORMAL;

}


bool kw_let::compile(compiler* c)
{
    // now add the variable name into the variables
    if (!c->has_variable(m_name))
    {
        m_variable = c->create_variable(m_name);
    }
    else
    {
        m_variable = c->get_variable(m_name);
    }

    if(m_indexed)
    {
        // --- Indexed Assignment Logic using the COPY instruction ---
        // This version uses 1-based indexing as requested.
        std::cout << "traversing" << std::endl;
        // Step 1: Evaluate the index expression (e.g., 2) into r0, then move to r128.
        // r128 will hold our final destination address.
        traverse_ast(128, m_index_seq->root(), c);

        std::cout << "generate mov reg129, m_variable" << std::endl;
        // Step 2: Get the base address of the target string variable 'a' into r129.
        (*c->generator()) << MOV() << reg(129) << m_variable;

        // Step 3: Calculate the final destination address in r128.
        // For 1-based indexing, the address is: base_address + index.
        std::cout << "adding 129 to 128" << std::endl;
        (*c->generator()) << ADD() << reg(128) << reg(129); // r128 = base_address + index

        // Step 4: Evaluate the RHS expression (e.g., "X"). The address of the string
        // literal is now in r0.
        std::cout << "evaluating RHS should go to reg0" << std::endl;
        sequence::compile(c);

        // Step 5: Calculate the source character's address in r0.
        // This is equivalent to: r0 = address_of("X") + 1 (to skip the length prefix).
        std::cout << "increasing reg0" << std::endl;
        (*c->generator()) << ADD() << reg(128) << token("0", token::type::TT_NUMBER); // COUNT: 1 byte to skip the first char

        (*c->generator()) << INC() << reg(0);



        std::cout << "copystuff" << std::endl;
        // Step 6: Generate the COPY instruction.
        // COPY <dest_addr_in_reg>, <src_addr_in_reg>, <count_immediate>
        (*c->generator()) << COPY()
                          << reg(128) // DEST: The calculated destination address
                          << reg(0)   // SRC: The source character's address
                          << token("1", token::type::TT_NUMBER); // COUNT: 1 byte
    }
    else
    {
        // Original logic for non-indexed assignment.
        sequence::compile(c);
        (*c->generator()) << MOV() << m_variable << reg(0);
    }
    return false;
}


