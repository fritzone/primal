#include "asm_compiler.h"
#include "util.h"
#include "exceptions.h"
#include "opcode.h"
#include "options.h"

#include <opcodes.h>

#include <iostream>
#include <iomanip>

using namespace primal;

void asm_compiler::generate_assembly_code(const primal::opcodes::opcode& opc, const std::vector<token>& tokens, std::vector<uint8_t>& result)
{

    // the opcode for COPY
    result.push_back(opc.bin());

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
            if(opc.family() == primal::opcodes::opcode_family::OF_JUMP)
            {
                throw syntax_error("Jump commands cannot jump to labels. Use goto for that purpose.");
            }
            else
            {
                throw syntax_error("Invalid assembly command parameter: " + t.data());
            }
        }

    }

    if(options::instance().generate_assembly())
    {
        std::cout << "[";
        for(auto b : result)
        {
            std::cout << " " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(b) ;
        }
        std::cout << " ]";
    }
}
