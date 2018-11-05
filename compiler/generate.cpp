#include "generate.h"

#include "opcodes.h"
#include "variable.h"
#include "registers.h"
#include "token.h"
#include "label.h"
#include "options.h"
#include "util.h"
#include "types.h"
#include "stringtable.h"

#include <iostream>
#include <limits>
#include <iomanip>
#include <cstring>
#include <map>

using namespace primal;

const size_t PRIMAL_HEADER_SIZE = 8;

std::map<const compiler*, std::shared_ptr<compiled_code>> compiled_code::compilers_codes;

generate::generate(compiler* c) : m_compiler(c)
{
    m_current_binseq_start = compiled_code::instance(m_compiler).location();
    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(5) << std::dec << compiled_code::instance(m_compiler).location() << ": ";
    }
}

generate::~generate()
{
    m_current_binseq_end = compiled_code::instance(m_compiler).location();
    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(30 - 10 * params_sent) << "# " << "L:" << (m_current_binseq_end - m_current_binseq_start) << " [";
        for(size_t i = m_current_binseq_start; i< m_current_binseq_end; i++)
        {
            uint8_t b = compiled_code::instance(m_compiler).bytecode().at(i);
            options::instance().asm_stream() << " " << std::setfill('0') << std::setw(2) << std::hex << std::uppercase  << static_cast<int>(b) ;
        }
        options::instance().asm_stream() << " ]";
        options::instance().asm_stream() << std::endl;
    }
}

generate &generate::operator<<(primal::opcodes::opcode &&opc)
{
    if(options::instance().generate_assembly()) { options::instance().asm_stream() << "(" << std::hex << static_cast<int>(opc.bin()) << ") " << std::setfill(' ') << std::left << std::setw(10) << opc.name() << " "; }

    compiled_code::instance(m_compiler).append(opc.bin());

    return *this;
}

generate &generate::operator<<(variable &&var)
{
    numeric_t a = var.location() * sizeof(numeric_t);
    auto address = htovm(a);

    if(options::instance().generate_assembly()) {
        std::stringstream ss;
        ss << "[" << address << "]";
        options::instance().asm_stream() << std::setfill(' ') << std::left << std::setw(10) << ss.str();
        params_sent ++;
    }

    // firstly we tell the VM the type of the data
    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(type_destination ::TYPE_MOD_MEM_IMM)));

    compiled_code::instance(m_compiler).append_number(address);

    return *this;
}

generate &generate::operator<<(std::shared_ptr<variable> var)
{
    return operator<<(std::forward<variable>(*var));
}

generate &generate::operator<<(reg &&r)
{
    if(options::instance().generate_assembly())
    {
        std::stringstream ss;
        ss << "$r" << static_cast<int>(r.idx());
        options::instance().asm_stream() << std::setfill(' ') << std::left << std::setw(10) << ss.str();
        params_sent ++;
    }

    // firstly we tell the VM the type of the data
    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_REG)));

    // and then the index of the register
    compiled_code::instance(m_compiler).append (static_cast<uint8_t>( r.idx() ));

    return *this;
}

generate &generate::operator<<(std::shared_ptr<opcodes::opcode> opc)
{
    return operator << (std::forward<opcodes::opcode>(*opc));
}

generate &generate::operator<<(const token &tok)
{
    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::left << std::setw(10) << tok.data();
        params_sent ++;
    }

    switch(tok.get_type())
    {
    case token::type::TT_NUMBER:
        {
            numeric_t tok_value = std::stoi(tok.data());

            // 0 is the indicator that the following is to be interpreted as a direct number
            compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_IMM)));

            // and the number goes out here
            compiled_code::instance(m_compiler).append_number(tok_value);
            break;
        }
    default:
        throw std::string("this token cannot be generated: " + tok.data());

    }
    return *this;
}

generate &generate::operator<<(const label& l)
{
    if(options::instance().generate_assembly()) { options::instance().asm_stream() << l.name(); }

    /* the label following this will be a simple number, send out immediate type modifier */
    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(type_destination::TYPE_MOD_IMM)));

    /* For the label encounters of this label we add a new value, being the current location of the code */
    compiled_code::instance(m_compiler).encountered(l);

    /* For now the label will go out in the code asa numeric value since if it was not declared yet
     * there is no way for us to know the location itself */
    for(size_t i=0; i<sizeof(numeric_t); i++)
    {
        compiled_code::instance(m_compiler).append (0xFF);
    }

    return *this;
}

generate &generate::operator<<(declare_label &&dl)
{
    if(options::instance().generate_assembly()) { options::instance().asm_stream() << ":" << dl.get_label().name(); }
    compiled_code::instance(m_compiler).declare_label(dl.get_label());
    return *this;
}

generate &generate::operator <<(type_destination td)
{
    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(td)));
    return *this;
}

generate &generate::operator <<(numeric_t v)
{
    if(options::instance().generate_assembly()) { options::instance().asm_stream() << v; }

    numeric_t vm_v = htovm(v);
    compiled_code::instance(m_compiler).append_number(vm_v);
}

compiled_code &compiled_code::instance(compiler* c)
{
    if(compilers_codes.count(c))
    {
        return *compilers_codes[c];
    }

    auto p = std::make_shared<compiled_code>(c);
    compilers_codes[c] = p;
    return *compilers_codes[c];
}

void compiled_code::append(uint8_t b)
{
    bytes.push_back(b);
}

void compiled_code::encountered(const label& l)
{
    if(bytes.size() > std::numeric_limits<uint32_t>::max())
    {
        throw "sorry mate, this application is too complex for me to compile";
    }

    if(label_encounters.count(l.name()) > 0)
    {
        label_encounters[l.name()].push_back(static_cast<uint32_t>(bytes.size()));
    }
    else
    {
        label_encounters[l.name()] = { static_cast<uint32_t>(bytes.size()) };
    }
}

void compiled_code::declare_label(const label& l)
{
    label_declarations[l.name()] = static_cast<uint32_t>(bytes.size());
}

void compiled_code::string_encountered(int strtbl_idx)
{
    if(string_encounters.count(strtbl_idx) > 0)
    {
        string_encounters[strtbl_idx].push_back(static_cast<uint32_t>(bytes.size()));
    }
    else
    {
        string_encounters[strtbl_idx] = { static_cast<uint32_t>(bytes.size()) };
    }
}

void compiled_code::finalize()
{
    // insert the offset of the string table bytes
    for(size_t i=0; i<sizeof(numeric_t); i++)
    {
        bytes.insert(bytes.begin(), 0xFF);
    }

    // insert the version 1.0 and .P to mark it as a primal compiled script
    bytes.insert(bytes.begin(), '0');bytes.insert(bytes.begin(), '1');bytes.insert(bytes.begin(), 'P');bytes.insert(bytes.begin(), '.');

    // finalize the labels
    for(const auto& ldecl : label_declarations)
    {
        for(const auto& lref : label_encounters[ldecl.first])
        {
            numeric_t dist_diff = ldecl.second - lref;
            // substract the size of a labels' address, as added in the generate to get the correct location
            numeric_t vm_ord = htovm(dist_diff) - sizeof(numeric_t);
            memcpy( &bytes[0] + lref, &vm_ord, sizeof(vm_ord));

            if(options::instance().generate_assembly())
            {
                options::instance().asm_stream() << "decl:" << ldecl.first << "(@" << ldecl.second << ") " << " ref found at " << lref << " patching to:" << vm_ord << std::endl;
            }
        }
    }

    // push a halt instruction at the end.
    bytes.push_back(0xFF);

    // fix the string table offset
    numeric_t l = bytes.size();
    numeric_t vm_l = htovm(l);
    memcpy( &bytes[4], &vm_l, sizeof(vm_l));

    // finalize the stringtable
    numeric_t strtblcnt = stringtable::instance().count();
    for(auto i = 0; i<strtblcnt; i++)
    {
        auto& e = stringtable::instance().e(i);
        uint8_t l = static_cast<uint8_t>(e.the_string.length());
        e.location = static_cast<numeric_t>(bytes.size());
        bytes.push_back(l);
        for(const auto& c : e.the_string)
        {
            bytes.push_back(static_cast<uint8_t>(c));
        }
    }

    for(const auto& s_e : string_encounters)
    {
        numeric_t i = s_e.first;
        auto& entry = stringtable::instance().e(i);
        numeric_t vm_loc = htovm(entry.location);
        for(const auto& e : s_e.second)
        {
            memcpy(&bytes[0] + e + PRIMAL_HEADER_SIZE, &vm_loc, sizeof(vm_loc));
        }
    }

    // all done theoretically
}

void compiled_code::append_number(numeric_t v)
{
    numeric_t nv = htovm(v);

    for(std::size_t i = 0; i< sizeof(v); i++)
    {
        append( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
    }
}

void compiled_code::destroy()
{
    if(m_compiler == nullptr)
    {
        return;
    }

    if(compilers_codes.count(m_compiler))
    {
        compilers_codes.erase(m_compiler);
    }
}
