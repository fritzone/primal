#ifndef TOKEN_H
#define TOKEN_H

#include <hal.h>

#include <vector>
#include <string>
#include <opcode.h>

namespace primal
{
/* This class represents a token */
    class token final
    {
    public:

        token() = default;

        /* What type a token is */
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
            TT_UNKNOWN              = 255
        };

        // Constructor
        token(const std::string& s, type t);
        token(char c, type t);

        // tells us from a simple language construct point of view if it is ok that this token
        // can be appended with the character of token type t2
        bool is_allowed_to_combine_token(char next);

        // tells us if it's ok to append "next" to the current token, when the previous token
        // is known. Used when determining negative or positive numbers
        bool is_allowed_to_combine_token(char next, const token& previos_token);

        // Adds a new character to this token
        void extend(char c);
        void set_type(type t);

        std::string data() const;
        void set_data(const std::string& d);

        type get_type() const;

        void set_extra_info(numeric_t v);
        numeric_t  get_extra_info() const;

        // will create a register from this token for easier compilation
        reg create_register() const;
        numeric_t to_number() const;

        bool is_register() const;
    public:

        // Gets the type of a character
        static type identify_type(char c);


        // will create a valid assembly <DST, SRC> pair for assembly commands supporting it
        static std::vector<token> identify_assembly_parameters(const std::vector<token>& tokens, const opcodes::opcode& opc);

    private:

        std::string m_data;
        type m_type = type::TT_UNKNOWN;
        numeric_t m_extra = -1;
    };

}


#endif // TOKEN_H
