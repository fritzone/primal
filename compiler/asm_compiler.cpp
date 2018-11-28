#include "asm_compiler.h"
#include "util.h"
#include "exceptions.h"
#include "opcode.h"
#include "opcodes.h"
#include "options.h"
#include "compiler.h"
#include "generate.h"
#include "label.h"
#include <numeric_decl.h>
#include <opcodes.h>

#include <iostream>
#include <iomanip>

using namespace primal;

void asm_compiler::generate_assembly_code(const primal::opcodes::opcode& opc, const std::vector<token>& tokens, std::vector<uint8_t>& result, compiler *c)
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
                        word_t v = tcopy.to_number();
                        word_t nv = htovm(v);

                        result.push_back(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_MEM_IMM_BYTE)));

                        for(std::size_t i = 0; i < word_size; i++)
                        {
                            result.push_back(*((reinterpret_cast<uint8_t *>(&nv) + i)));
                        }
                    }
                    else
                    {
                        // let's see if there is a - or a + in the td
                        int curr_op = 0; // no operations, 1 : +, 2 : -
                        auto oppos = td.find_first_of('-') ;
                        if(oppos != std::string::npos)
                        {
                            curr_op = 2;

                        }
                        else
                        {
                            oppos = td.find_first_of('+') ;
                        }
                        if(oppos != std::string::npos)
                        {
                            curr_op = 1;
                        }

                        if(curr_op == 0) throw syntax_error("Invalid memory access: " + td);

                        std::string reg = td.substr(0, oppos);
                        std::string distance = td.substr(oppos); // this will take in + or - too
                        token t_reg(reg, token::type::TT_REGISTER);

                        uint8_t op = distance[0];
                        distance = distance.substr(1);  // skip the + or -, it does not matter here

                        if(util::is_number(distance))    // is the distance a number
                        {
                            int dist = std::stoi(distance);

                            if(t_reg.is_register())
                            {
                                auto r = t_reg.create_register();
                                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination ::TYPE_MOD_MEM_REG_IDX_OFFS)));
                                result.push_back(r.idx());
                                result.push_back(op);
                                word_t nv = htovm(static_cast<word_t>(dist));

                                for(std::size_t i = 0; i< word_size; i++)
                                {
                                    result.push_back( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
                                }
                            }
                        }
                        else    // is the distance a register?
                        {
                            token t_reg2(distance, token::type::TT_REGISTER);
                            if(t_reg2.is_register() && t_reg.is_register())
                            {
                                auto r = t_reg.create_register();
                                auto r2 = t_reg2.create_register();
                                result.push_back(static_cast<uint8_t>(util::to_integral(type_destination ::TYPE_MOD_MEM_REG_IDX_REG_OFFS)));
                                result.push_back(r.idx());
                                result.push_back(op);
                                result.push_back(r2.idx());
                            }
                        }

                    }
                }
                else
                {
                    word_t v = t.to_number();
                    word_t nv = htovm(v);

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
                word_t v = t.to_number();
                word_t nv = htovm(v);

                for(std::size_t i = 0; i< sizeof(v); i++)
                {
                    result.push_back( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
                }

                break;
            }

            default:
            if(opc.family() == primal::opcodes::opcode_family::OF_JUMP)
            {
                label l(c->get_source());
                l.set_name(tokens[0].data());

                result.push_back(util::to_integral(type_destination ::TYPE_MOD_IMM));

                // notifying the label
                compiled_code::instance(c).encountered(l, true, result.size());

                // the label location
                for(size_t i=0; i<word_size; i++)
                {
                    result.push_back(0xFF);
                }
            }
            else
            {
                throw syntax_error("Invalid assembly command parameter: " + t.data());
            }
        }
    }

    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(20) << "# " << "L:" << result.size() << " [";
        for(auto b : result)
        {
            options::instance().asm_stream() << " " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(b) ;
        }
        options::instance().asm_stream() << " ]";
    }
}
