#include <MOV.h>
#include <token.h>

#include <iostream>
#include <vector>
#include <util.h>

std::vector<uint8_t> compile_MOV(std::vector<token>& tokens)
{
    if(tokens.size() != 2)
    {
        throw syntax_error("incorrect MOV");
    }

    std::vector<uint8_t> result;

    result.push_back(opcodes::MOV().bin());
    switch(tokens[0].get_type())
    {
        case token::type::TT_REGISTER:
        {
            reg r = tokens[0].create_register();
            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG)));
            result.push_back(r.idx());
            break;
        }

        default:
            throw syntax_error("Invalid MOV target");
    }

    switch(tokens[1].get_type())
    {
        case token::type::TT_NUMBER:
        {
            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_IMM)));
            numeric_t v = tokens[1].to_number();
            numeric_t nv = htovm(v);

            for(std::size_t i = 0; i< sizeof(v); i++)
            {
                result.push_back( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
            }

            break;
        }

        default:
            throw syntax_error("Invalid MOV source");
    }


    return result;
}

