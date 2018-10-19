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
                if(!util::is_number(t.data()))
                {
                    token tcopy = t;
                    auto td = tcopy.data();
                    bool byte_access = false;

                    if(td.length() >= 1 && td[0] == '@')
                    {
                        byte_access = true;
                        tcopy.set_data(td.substr(1));
                    }

                    if(tcopy.is_register())
                    {
                        auto r = tcopy.create_register();
                        result.push_back(static_cast<uint8_t>(util::to_integral(byte_access ? type_destination ::TYPE_MOD_MEM_REG_BYTE : type_destination::TYPE_MOD_MEM_REG_IDX)));
                        result.push_back(r.idx());
                    }
                    else
                    if(util::is_number(tcopy.data()))
                    {
                        numeric_t v = tcopy.to_number();
                        numeric_t nv = htovm(v);

                        result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_MEM_IMM_BYTE)));

                        for(std::size_t i = 0; i< sizeof(v); i++)
                        {
                            result.push_back(*((reinterpret_cast<uint8_t *>(&nv) + i)));
                        }
                    }
                    else
                    {
                        throw syntax_error("Invalid memory access");
                    }
                }
                else
                {
                    numeric_t v = t.to_number();
                    numeric_t nv = htovm(v);

                    result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_MEM_IMM)));

                    for(std::size_t i = 0; i< sizeof(v); i++)
                    {
                        result.push_back(*((reinterpret_cast<uint8_t *>(&nv) + i)));
                    }
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
