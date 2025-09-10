#include "function.h"
#include "util.h"
#include "source.h"
#include "parser.h"
#include "compiler.h"
#include "opcodes.h"
#include "sequence.h"
#include "generate.h"
#include "options.h"
#include "label.h"
#include "variable.h"

#include <kw_end.h>

using namespace primal;
using namespace primal::opcodes;

std::map<std::string, std::shared_ptr<fun>> fun::m_functions;

void fun::reset()
{
    m_functions.clear();
}
#if 0
void fun::identify_parameters(std::vector<token> &tokens)
{
    entity_type current_parameter_type = entity_type::ET_UNKNOWN;

    while(!tokens.empty() && tokens[0].get_type() == token::type::TT_OPEN_PARENTHESES) tokens.erase(tokens.begin());

    auto tt0_type = get_entity_type(tokens[0].data());

    if(tt0_type == entity_type::ET_UNKNOWN)
    {
        current_parameter_type = entity_type::ET_NUMERIC;
    }
    else
    if(tt0_type == entity_type::ET_ELLIPSIS)
    {
        m_variadic = true;
        return;
    }
    else
    {
        // remove the first one
        tokens.erase(tokens.begin());
        if(tokens.empty())
        {
            return ;
        }
    }

    for(const auto& t : tokens)
    {
        if(t.get_type() != token::type::TT_COMMA && t.get_type() != token::type::TT_CLOSE_PARENTHESES && t.get_type() != token::type::TT_OPEN_PARENTHESES )
        {
            entity_type type_2 = get_entity_type(t.data());
            if(type_2 == entity_type::ET_ELLIPSIS)
            {
                m_variadic = true;
            }
            else
            if(type_2 == entity_type::ET_UNKNOWN) // still a parameter name
            {
                m_parameters.push_back({t.data(), current_parameter_type});
                variable::introduce_name(t.data(), current_parameter_type, entity_origin::EO_PARAMETER);
            }
            else    // switched over to a new type
            {
                current_parameter_type = type_2;
            }
        }
    }

}
#endif

void fun::identify_parameters(std::vector<token> &tokens)
{
    m_parameters.clear();
    if (tokens.empty()) {
        return; // No parameters
    }

    if (tokens.size() == 1 && tokens[0].data() == "...") {
        m_variadic = true;
        return;
    }

    entity_type current_parameter_type = entity_type::ET_UNKNOWN;

    for(const auto& t : tokens)
    {
        if (t.get_type() == token::type::TT_COMMA) {
            continue; // Ignore commas
        }

        // Check if the token is a type specifier (e.g., "string", "number").
        entity_type type_check = get_entity_type(t.data());

        if (type_check != entity_type::ET_UNKNOWN && type_check != entity_type::ET_ELLIPSIS)
        {
            // It's a new type specifier.
            current_parameter_type = type_check;
        }
        else if (t.get_type() == token::type::TT_IDENTIFIER)
        {
            // It's a parameter name.
            if (current_parameter_type == entity_type::ET_UNKNOWN)
            {
                // This is an error: a parameter name was found before any type was specified.
                throw syntax_error("Parameter '" + t.data() + "' in function '" + m_name + "' is missing a type specifier (e.g., 'int' or 'string').");
            }
            // Add the parameter with its determined type.
            m_parameters.push_back({t.data(), current_parameter_type});
            variable::introduce_name(t.data(), current_parameter_type, entity_origin::EO_PARAMETER);
        }
        else
        {
            throw syntax_error("Unexpected token '" + t.data() + "' in parameter list for function '" + m_name + "'.");
        }
    }
}

std::string fun::name() const { return m_name; }

void fun::parse()
{
    // npw resolve the parameters, etc...
    parser p;
    std::string last;
    auto seqs = p.parse(m_src,
            [&](std::string s)
            {
                return util::to_upper(s) == kw_end::N;
            },
    last);
    m_body = std::get<0>(seqs);
}

bool fun::compile(compiler* c)
{
    label fun_label = label::create(c->get_source());
    fun_label.set_name(m_name);
    (*c->generator()) << declare_label(fun_label);

    set_address(compiled_code::instance(c).location());

    // now the header for the function
    (*c->generator()) << PUSH() << reg(254);                 // save R254 somewhere
    (*c->generator()) << MOV()  << reg(254) << reg(255);     // save the SP into R254
    (*c->generator()) << SUB()  << reg(255) <<  type_destination::TYPE_MOD_IMM <<             // Decrease the stack pointer to skip the pushed r254 and return address
#if TARGET_ARCH == 32
        (8);                                        // For 32 bit builds we decrease with 8
#else
        (16);                                       // For 64 bit builds decrease with 16
#endif

    if(!is_extern())
    {
        for(const auto& seq : m_body)
        {
            seq->compile(c);
        }
    }
    else
    {
        //compiled_code::instance(c).string_encountered(m_name);

        (*c->generator()) << MOV() << reg(249) << reg(250);
        (*c->generator()) << SUB() << reg(249) << type_destination::TYPE_MOD_IMM  << (20);
        (*c->generator()) << INTR() << type_destination::TYPE_MOD_IMM << (2);
    }

    (*c->generator()) << MOV() << reg(255) << reg(254);     // Restore the stack pointer
    (*c->generator()) << POP() << reg(254);                 // restore R254 to what it was before

    // and then return to the caller
    (*c->generator()) << RET();
    return true;
}

parameter *fun::get_parameter(const std::string &name)
{
    auto i = std::find_if( m_parameters.begin(), m_parameters.end(),
                           [name](const parameter& element){ return element.name == name;} );
    if( i!= m_parameters.end())
    {
        return &(*i);
    }
    return nullptr;
}

int fun::get_parameter_index(const parameter *p)
{
    auto i = std::find_if( m_parameters.begin(), m_parameters.end(),
                           [p](const parameter& element){ return element.name == p->name;} );
    if( i!= m_parameters.end())
    {
        return static_cast<int>(i - m_parameters.begin()) + 1;
    }
    return 0;
}


const std::map<std::string, std::shared_ptr<fun>>& fun::get_functions() {
    return m_functions;
}

bool fun::is_extern() const
{
    return m_extern;
}

void fun::set_extern(bool newExtern)
{
    m_extern = newExtern;
}
