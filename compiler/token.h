#ifndef TOKEN_H
#define TOKEN_H

#include <string>

#include <hal.h>

/* This class represents a token */
class token
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
    type get_type() const;

    // will create a register from this token for easier compilation
    reg create_register() const;
    numeric_t to_number() const;
public:

    // Gets the type of a character
    static type identify_type(char c);

private:

    std::string m_data;
    type m_type;
};

#endif // TOKEN_H
