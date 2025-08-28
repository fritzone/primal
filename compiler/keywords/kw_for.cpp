#include "kw_for.h"
#include "kw_next.h"
#include "parser.h"
#include "opcodes.h"
#include "generate.h"
#include "compiler.h"
#include "label.h"
#include "util.h"
#include "exceptions.h"
#include "variable.h"

#include <options.h>


using namespace primal;
using namespace primal::opcodes;

sequence::prepared_type kw_for::prepare(std::vector<token>& tokens) {
    if (tokens.size() < 4) {
        throw syntax_error("Incomplete FOR statement");
    }

    // 1. Get the iterator variable name
    m_iterator_name = tokens[0].data();
    tokens.erase(tokens.begin()); // remove iterator

    // 2. Check for '='
    if (tokens[0].data() != "=") {
        throw syntax_error("Expected '=' in FOR statement");
    }
    tokens.erase(tokens.begin()); // remove '='

    // 3. Parse the initial value expression (up to TO)
    std::vector<token> initial_tokens;
    auto it_to = std::find_if(tokens.begin(), tokens.end(), [](const token& t) {
        return util::to_upper(t.data()) == "TO";
    });
    if (it_to == tokens.end()) {
        throw syntax_error("Expected 'TO' in FOR statement");
    }
    std::copy(tokens.begin(), it_to, std::back_inserter(initial_tokens));
    tokens.erase(tokens.begin(), it_to + 1);

    // 4. Parse the end value expression (up to STEP or end)
    std::vector<token> end_tokens;
    auto it_step = std::find_if(tokens.begin(), tokens.end(), [](const token& t) {
        return util::to_upper(t.data()) == "STEP";
    });
    std::copy(tokens.begin(), it_step, std::back_inserter(end_tokens));
    tokens.erase(tokens.begin(), it_step);

    // 5. Parse the optional step value expression
    std::vector<token> step_tokens;
    if (!tokens.empty() && util::to_upper(tokens[0].data()) == "STEP") {
        tokens.erase(tokens.begin()); // remove STEP
        step_tokens = tokens;
    } else {
        // Default step is 1
        step_tokens.push_back(token("1", token::type::TT_NUMBER));
    }

    // Create sequences and build ASTs for each part
    auto init_seq_tokens = parser::shuntyard(initial_tokens);
    m_initial_seq = sequence::create(init_seq_tokens, source::get());
    ast::build_ast(init_seq_tokens, m_initial_seq->root());


    auto end_seq_tokens = parser::shuntyard(end_tokens);
    m_end_seq = sequence::create(end_seq_tokens, source::get());
    ast::build_ast(end_seq_tokens, m_end_seq->root());


    auto step_seq_tokens = parser::shuntyard(step_tokens);
    m_step_seq = sequence::create(step_seq_tokens, source::get());;
    ast::build_ast(step_seq_tokens, m_step_seq->root());

    // 6. Parse the loop body until NEXT
    parser p;
    std::string last;
    auto seqs = p.parse(m_src, [&](std::string s) {
        return util::to_upper(s) == kw_next::N;
    }, last);
    m_for_body = std::get<0>(seqs);

    return sequence::prepared_type::PT_CONSUMED;
}

bool kw_for::compile(compiler* c)
{


    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << "===" << m_string_seq << "===" << std::endl;
    }

    // Ensure the iterator variable exists
    if (!c->has_variable(m_iterator_name)) {
        throw syntax_error("FOR loop iterator '" + m_iterator_name + "' is not declared.");
    }
    m_iterator = c->get_variable(m_iterator_name);

    // --- Initialization ---
    // let i = initial_value
    m_initial_seq->compile(c);
    (*c->generator()) << MOV() << m_iterator << reg(0);

    // --- Loop structure ---
    label lbl_loop_start = label::create(c->get_source());
    label lbl_loop_body = label::create(c->get_source());
    label lbl_loop_end = label::create(c->get_source());

    // --- Condition Check ---
    (*c->generator()) << declare_label(lbl_loop_start);
    // Evaluate end value and store it (e.g., in r128)
    traverse_ast(128, m_end_seq->root(), c);

    (*c->generator()) << MOV() << reg(0) << m_iterator;
    // Compare: i <= end
    (*c->generator()) << LTE() << reg(0) << reg(128);
    // Jump to body if true
    (*c->generator()) << DJT() << lbl_loop_body;
    // Jump to end if false
    (*c->generator()) << DJMP() << lbl_loop_end;

    // --- Loop Body ---
    (*c->generator()) << declare_label(lbl_loop_body);
    for (const auto& seq : m_for_body) {
        seq->compile(c);
    }

    // --- Increment Step ---
    // i = i + step_value

    traverse_ast(128, m_step_seq->root(), c);
    (*c->generator()) << ADD() << m_iterator << reg(128);

    // --- Jump back to start ---
    (*c->generator()) << DJMP() << lbl_loop_start;

    // --- End of Loop ---
    (*c->generator()) << declare_label(lbl_loop_end);

    return true;
}
