#include "token.h"
#include "util.h"
#include "exceptions.h"

using namespace primal;

token::token(const std::string &s, token::type t) : m_data(s), m_type(t) {}
token::token(char c, token::type t) : token("", t) { extend(c); }

bool token::is_allowed_to_combine_token(char next, const token& previos_token)
{
    token::type t_prev = previos_token.get_type();
    if(is_allowed_to_combine_token(next)) {return true;}
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
    return (    (m_type == token::type::TT_IDENTIFIER && t2 == token::type::TT_NUMBER)
            || (m_type == token::type::TT_IDENTIFIER && t2 == token::type::TT_IDENTIFIER)
            || (m_type == token::type::TT_NUMBER && t2 == token::type::TT_NUMBER)
            || (m_type == token::type::TT_DOT && t2 == token::type::TT_DOT)
            || (m_type == token::type::TT_DOT && t2 == token::type::TT_NUMBER)
            || (m_type == token::type::TT_NUMBER && t2 == token::type::TT_DOT)
            || (m_type == token::type::TT_ASSIGNMENT && t2 == token::type::TT_ASSIGNMENT)
            || (m_type == token::type::TT_EXCLAMATION && t2 == token::type::TT_ASSIGNMENT)
            || (m_type == token::type::TT_REGISTER && (next == 'r' || ::isdigit(next)))
            );
}

void token::extend(char c) { m_data += c; }
void token::set_type(type t) { m_type = t; }
std::string token::data() const { return m_data; }
token::type token::get_type() const { return m_type; }

// MODIFIED: identify_type now recognizes brackets
token::type token::identify_type(char c)
{
    if (util::is_operator(c)) return token::type::TT_OPERATOR;
    if (util::is_integer(c)) return token::type::TT_NUMBER;
    if (util::is_identifier_char(c)) return token::type::TT_IDENTIFIER;
    if (c == '(') return token::type::TT_OPEN_PARENTHESES;
    if (c == ')') return token::type::TT_CLOSE_PARENTHESES;
    if (c == '[') return token::type::TT_OPEN_BRACKET;   // NEW
    if (c == ']') return token::type::TT_CLOSE_BRACKET;  // NEW
    if (c == '=') return token::type::TT_ASSIGNMENT;
    if (c == '!') return token::type::TT_EXCLAMATION;
    if (c == '$') return token::type::TT_REGISTER;
    if (c == ':') return token::type::TT_LABEL;
    if (c == '#') return token::type::TT_COMMENT_LINE;
    if (c == '"') return token::type::TT_STRING;
    if (c == ',') return token::type::TT_COMMA;
    if (c == '.') return token::type::TT_DOT;
    return token::type::TT_UNKNOWN;
}

reg token::create_register() const
{
    if(   (m_data.length() > 3 && (m_data[0] != '$' && m_data[1] != 'r'))
        ||(m_data.length() >= 3 && (m_data[0] != '$' && m_data[1] != 's' && m_data[2] != 'p'))
        )
    {
        throw syntax_error("Invalid register: " + m_data);
    }
    uint8_t ridx = 0;
    if(m_data == "$sp") { ridx = 255; } else { ridx = static_cast<uint8_t>(std::stoi(m_data.substr(2))); }
    return reg( ridx );
}

word_t token::to_number() const { return std::stoi(m_data); }
void token::set_data(const std::string &d) { m_data = d; }
void token::set_extra_info(word_t v) { m_extra = v; }
word_t token::get_extra_info() const { return m_extra; }

std::vector<token> token::identify_assembly_parameters(const std::vector<token> &tokens, const opcodes::opcode &opc)
{
    std::vector<token> work_tokens;
    for (size_t i = 0; i < tokens.size(); i++)
    {
        token t = tokens[i];
        if (t.data() == "[")
        {
            token new_entry;
            new_entry.set_type(token::type::TT_ASM_MEMORY_ADDRESS);
            std::string token_data;
            bool valid_mem_entry = false;
            for (size_t j = i + 1; j < tokens.size(); j++, i++)
            {
                if (tokens[j].data() != "]")
                {
                    token_data += tokens[j].data();
                }
                else
                {
                    valid_mem_entry = true;
                    i++;
                    break;
                }
            }

            if (!valid_mem_entry)
            {
                throw syntax_error("Invalid memory addres syntax");
            }

            new_entry.set_data(token_data);
            work_tokens.push_back(new_entry);
        }
        else
        {
            if (t.data() == "@")
            {
                auto &l = work_tokens.back();
                l.set_type(token::type::TT_ASM_REG_SUBBYTE);
                i++;
                if (i < tokens.size())
                {
                    int subbyte_nr = std::stoi(tokens[i].data());
                    if (subbyte_nr < static_cast<int>(word_size))
                    {
                        l.set_extra_info(subbyte_nr);
                    }
                    else
                    {
                        throw syntax_error("Invalid register subbyte index");
                    }
                }
                else
                {
                    throw syntax_error("Incomplete register subbyte operation");
                }
            }
            else
            {
                work_tokens.push_back(t);
            }
        }
    }
    if (work_tokens.size() != opc.paramcount())
    {
        throw syntax_error("Invalid assembler statement");
    }
    return work_tokens;
}
bool token::is_register() const
{
    if(m_data.length() > 2 && (m_data[0] == '$' && m_data[1] == 'r'))
    {
        std::string afterd = m_data.substr(2);
        if(util::is_number(afterd))
        {
            auto n = std::stoi(afterd);
            return n < 256;
        }
    }
    return false;
}
std::string token::token_vector_to_string(const std::vector<primal::token>& v)
{
    std::string result;
    std::for_each(v.begin(), v.end(), [&result](const primal::token& t) {result += t.data();});
    return result;
}

void token::remove_commas(std::vector<token> &tokens) {
    tokens.erase(
        std::remove_if(tokens.begin(), tokens.end(),
                       [](const token& t) {
                           return t.get_type() == token::type::TT_COMMA
                                  && t.data() == ",";
                       }),
        tokens.end()
        );
}
