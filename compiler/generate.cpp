#include "generate.h"

#include "opcodes.h"
#include "variable.h"
#include "registers.h"
#include "token.h"
#include "label.h"
#include "options.h"
#include "util.h"

#include <iostream>
#include <limits>

using namespace opcodes;

std::map<const compiler*, std::shared_ptr<compiled_code>> compiled_code::compilers_codes;

generate::generate(compiler* c) : m_compiler(c)
{
    if(options::instance().generate_assembly()) { std::cout << compiled_code::instance(m_compiler).location() << ": "; }
}

generate::~generate()
{
    if(options::instance().generate_assembly()) { std::cout << std::endl; }
}

generate &generate::operator<<(opcode &&opc)
{
    if(options::instance().generate_assembly()) { std::cout << opc.name() << " "; }

    compiled_code::instance(m_compiler).append(opc.bin());

    return *this;
}

generate &generate::operator<<(variable &&var)
{
    if(options::instance().generate_assembly()) { std::cout << "[" << var.location() << "]" << " "; }

    // firstly we tell the VM the type of the data
    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(type_destination ::TYPE_MOD_VAR)));

    compiled_code::instance(m_compiler).append_number(var.location());

    return *this;
}

generate &generate::operator<<(std::shared_ptr<variable> var)
{
    return operator<<(std::forward<variable>(*var));
}

generate &generate::operator<<(reg &&r)
{
    if(options::instance().generate_assembly()) { std::cout << "r" << static_cast<int>(r.idx()) << " "; }

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
    if(options::instance().generate_assembly()) { std::cout << tok.data() << " "; }

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
    if(options::instance().generate_assembly()) { std::cout << l.name(); }

    /* For the label encounters of this label we add a new value, being the current location of the code */
    compiled_code::instance(m_compiler).encountered(l);

    /* For now the label will go out in the code asa 4 byte long value since if it was not declared yet
     * there is no way for us to know the location itself */
    compiled_code::instance(m_compiler).append (0xFF);
    compiled_code::instance(m_compiler).append (0xFF);
    compiled_code::instance(m_compiler).append (0xFF);
    compiled_code::instance(m_compiler).append (0xFF);

    return *this;
}

generate &generate::operator<<(declare_label &&dl)
{
    if(options::instance().generate_assembly()) { std::cout << ":" << dl.get_label().name(); }
    compiled_code::instance(m_compiler).declare_label(dl.get_label());
    return *this;
}

generate &generate::operator<<(type_destination td)
{
    if(options::instance().generate_assembly())
    {
        switch(td)
        {
            case type_destination::TYPE_MOD_REG: std::cout << "R"; break;
            case type_destination::TYPE_MOD_VAR: std::cout << "V"; break;
            case type_destination::TYPE_MOD_IMM: std::cout << "I"; break;
            case type_destination::TYPE_MOD_MEM_IMM: std::cout << "M"; break;
            default: std::cout << "?";
        }
    }

    compiled_code::instance(m_compiler).append(static_cast<uint8_t>(util::to_integral(td)));
    return *this;
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

void compiled_code::finalize()
{
    for(const auto& ldecl : label_declarations)
    {
        for(const auto& lref : label_encounters[ldecl.first])
        {
            std::cout << "decl:" << ldecl.first << "(@" << ldecl.second << ") " << " found at " << lref << std::endl;
        }
    }
}

void compiled_code::append_number(numeric_t v)
{
    numeric_t nv = htovm(v);

    for(std::size_t i = 0; i< sizeof(v); i++)
    {
        append( * ((reinterpret_cast<uint8_t *>(&nv) + i ) ));
    }
}
