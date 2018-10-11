#include "token.h"
#include "util.h"

token::token(const std::string &s, token::type t) : m_data(s), m_type(t)
{
}

token::token(char c, token::type t) : token("", t)
{
    extend(c);
}

bool token::is_allowed_to_combine_token(char next, const token& previos_token)
{
    token::type t_prev = previos_token.get_type();
    if(is_allowed_to_combine_token(next)) {return true;}

    // see for unary operator
    bool unary = ( (m_data == "-" || m_data == "+") && t_prev != token::type::TT_NUMBER && t_prev != token::type::TT_IDENTIFIER && t_prev != token::type::TT_CLOSE_PARENTHESES);
    bool register_type = (!previos_token.data().empty() && (previos_token.data()[0] == '$' && m_data == "r"));

    return unary || register_type;
}

bool token::is_allowed_to_combine_token(char next)
{
    if(m_type == token::type::TT_LABEL && (::isalnum(next) || next == '_'))
    {
        return true;
    }

    token::type t2 = identify_type(next);

    return ( (m_type == token::type::TT_IDENTIFIER && t2 == token::type::TT_NUMBER)
             || (m_type == token::type::TT_IDENTIFIER && t2 == token::type::TT_IDENTIFIER)
             || (m_type == token::type::TT_NUMBER && t2 == token::type::TT_NUMBER)
             || (m_type == token::type::TT_ASSIGNMENT && t2 == token::type::TT_ASSIGNMENT)
             || (m_type == token::type::TT_EXCLAMATION && t2 == token::type::TT_ASSIGNMENT)
             || (m_type == token::type::TT_REGISTER && (next == 'r' || ::isdigit(next)))
    );
}


void token::extend(char c)
{
    m_data += c;
}

void token::set_type(token::type t)
{
    m_type = t;
}

std::string token::data() const
{
    return m_data;
}

token::type token::get_type() const
{
    return m_type;
}

token::type token::identify_type(char c)
{
    if (util::is_operator(c)) return token::type::TT_OPERATOR;
    if (util::is_integer(c)) return token::type::TT_NUMBER;
    if (util::is_identifier_char(c)) return token::type::TT_IDENTIFIER;
    if (c == '(') return token::type::TT_OPEN_PARENTHESES;
    if (c == ')') return token::type::TT_CLOSE_PARENTHESES;
    if (c == '=') return token::type::TT_ASSIGNMENT;
    if (c == '!') return token::type::TT_EXCLAMATION;
    if (c == '$') return token::type::TT_REGISTER;
    if (c == ':') return token::type::TT_LABEL;

    return token::type::TT_UNKNOWN;
}

reg token::create_register() const
{
    if(   (m_data.length() < 3 || (m_data[0] != '$' && m_data[1] != 'r'))
        ||(m_data.length() < 3 || (m_data[0] != '$' && m_data[1] != 's' && m_data[2] != 'p'))
      )
    {
        throw syntax_error("Invalid register: " + m_data);
    }
    uint8_t ridx = 0;
    if(m_data == "$sp") { ridx = 255; } else { ridx = static_cast<uint8_t>(std::stoi(m_data.substr(2))); }

    return reg( ridx );
}

numeric_t token::to_number() const
{
    return std::stoi(m_data);
}

void token::set_data(const std::string &d)
{
    m_data = d;
}
