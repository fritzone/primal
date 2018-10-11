#include <MOV.h>
#include <token.h>

#include <iostream>
#include <vector>
#include <util.h>

std::vector<uint8_t> compile_MOV(std::vector<token>& tokens)
{
    std::vector<token> work_tokens;

    // see if any of the tokens are of memory address type
    for(size_t i = 0; i<tokens.size(); i++)
    {
        token t = tokens[i];
        if(t.data() == "[")
        {
            token new_entry;
            new_entry.set_type(token::type::TT_ASM_MEMORY_ADDRESS);
            std::string token_data = "";
            bool valid_mem_entry = false;
            for(size_t j = i+1; j<tokens.size(); j++, i++)
            {
                if(tokens[j].data() != "]")
                {
                    token_data += tokens[j].data();
                }
                else
                {
                    valid_mem_entry = true;
                    i++;    // skip "]" in the initial token list
                    break;
                }
            }

            if(!valid_mem_entry)
            {
                throw syntax_error("Invalid memory addres in MOV");
            }

            new_entry.set_data(token_data);
            work_tokens.push_back(new_entry);
        }
        else
        {
            work_tokens.push_back(t);
        }
    }
    if(work_tokens.size() != 2)
    {
        throw syntax_error("incorrect MOV");
    }

    std::vector<uint8_t> result;

    result.push_back(opcodes::MOV().bin());
    switch(work_tokens[0].get_type())
    {
        case token::type::TT_REGISTER:
        {
            reg r = work_tokens[0].create_register();
            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG)));
            result.push_back(r.idx());
            break;
        }

        case token::type::TT_ASM_MEMORY_ADDRESS:
        {
            numeric_t v = work_tokens[0].to_number();
            numeric_t nv = htovm(v);

            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_MEM_IMM)));

            for(std::size_t i = 0; i< sizeof(v); i++)
            {
                result.push_back(*((reinterpret_cast<uint8_t *>(&nv) + i)));
            }
            break;
        }

        default:
            throw syntax_error("Invalid MOV target");
    }

    switch(work_tokens[1].get_type())
    {
        case token::type::TT_NUMBER:
        {
            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_IMM)));
            numeric_t v = work_tokens[1].to_number();
            numeric_t nv = htovm(v);

            for(std::size_t i = 0; i< sizeof(v); i++)
            {
                result.push_back( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
            }

            break;
        }

        case token::type::TT_REGISTER:
        {
            reg r = work_tokens[1].create_register();
            result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG)));
            result.push_back(r.idx());
            break;
        }


        default:
            throw syntax_error("Invalid MOV source");
    }


    return result;
}

