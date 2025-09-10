#ifndef TOKEN_H
#define TOKEN_H

#include <numeric_decl.h>
#include <registers.h>
#include <vector>
#include <string>
#include <opcode.h>

namespace primal
{
class token final
{
public:
    token() = default;
    enum class type
    {
        TT_OPERATOR             = 1,
        TT_IDENTIFIER           = 2,
        TT_NUMBER               = 3,
        TT_OPEN_PARENTHESES     = 4,
        TT_CLOSE_PARENTHESES    = 5,
        TT_VARIABLE             = 6,
        TT_ASSIGNMENT           = 7,
        TT_COMPARISON           = 8,
        TT_EXCLAMATION          = 9,
        TT_REGISTER             = 10,
        TT_LABEL                = 11,
        TT_ASM_MEMORY_ADDRESS   = 12,
        TT_ASM_REG_SUBBYTE      = 13,
        TT_COMMENT_LINE         = 14,
        TT_STRING               = 15,
        TT_COMMA                = 16,
        TT_FUNCTION_CALL        = 17,
        TT_DOT                  = 18,
        TT_LOGICAL              = 19,
        TT_OPEN_BRACKET         = 20,
        TT_CLOSE_BRACKET        = 21,
        TT_UNKNOWN              = 255
    };
    token(const std::string& s, type t);
    token(char c, type t);
    bool is_allowed_to_combine_token(char next);
    bool is_allowed_to_combine_token(char next, const token& previos_token);
    void extend(char c);
    void set_type(type t);
    std::string data() const;
    void set_data(const std::string& d);
    type get_type() const;
    void set_extra_info(word_t v);
    word_t  get_extra_info() const;
    reg create_register() const;
    word_t to_number() const;
    bool is_register() const;
public:
    static type identify_type(char c);
    static std::vector<token> identify_assembly_parameters(const std::vector<token>& tokens, const opcodes::opcode& opc);
    static std::string token_vector_to_string(const std::vector<primal::token>& v);
    static void remove_commas(std::vector<token>& tokens);
private:
    std::string m_data;
    type m_type = type::TT_UNKNOWN;
    word_t m_extra = -1;
};
}

#endif
