#include "asm_compiler.h"
#include "util.h"

void asm_compiler::generate_assembly_code(const std::vector<token>& tokens, std::vector<uint8_t>& result)
{

    for(const token &t : tokens)
    {
        switch(t.get_type())
        {
            case token::type::TT_REGISTER:
            {
                reg r = t.create_register();
                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG)));
                result.push_back(r.idx());
                break;
            }

            case token::type::TT_ASM_MEMORY_ADDRESS:
            {
                numeric_t v = t.to_number();
                numeric_t nv = htovm(v);

                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_MEM_IMM)));

                for(std::size_t i = 0; i< sizeof(v); i++)
                {
                    result.push_back(*((reinterpret_cast<uint8_t *>(&nv) + i)));
                }
                break;
            }

            case token::type::TT_ASM_REG_SUBBYTE:
            {
                reg r = t.create_register();
                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG_BYTE0) + t.get_extra_info()) );
                result.push_back(r.idx());
                break;
            }

            case token::type::TT_NUMBER:
            {
                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_IMM)));
                numeric_t v = t.to_number();
                numeric_t nv = htovm(v);

                for(std::size_t i = 0; i< sizeof(v); i++)
                {
                    result.push_back( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
                }

                break;
            }

            default:
                throw syntax_error("Invalid assembly command parameter");
        }

    }
}
