#include "kw_return.h"
#include "parser.h"
#include "opcodes.h"
#include "generate.h"
#include "compiler.h"
#include "util.h"

using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_return::prepare(std::vector<token>& tokens) {
    // This method parses the expression that follows the 'return' keyword.
    // It builds an AST for the expression, which will be compiled later.
    // The base sequence class's `prepare` is not called, so we do it manually here.

    if (tokens.empty()) {
        // A return statement with no value. We can treat this as `return 0`.
        tokens.push_back(token("0", token::type::TT_NUMBER));
    }

    // Use the parser's shuntyard and build_ast to process the expression.
    auto rpn_tokens = parser::shuntyard(tokens);
    ast::build_ast(rpn_tokens, root());

    // This keyword consumes the rest of the line.
    return sequence::prepared_type::PT_CONSUMED;
}

bool kw_return::compile(compiler* c) {
    // 1. Compile the return expression.
    // This traverses the AST we built in `prepare()` and generates bytecode.
    // By convention, the final result of the expression is placed in register r0.
    sequence::compile(c);

    return true;
}
