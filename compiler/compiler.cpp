#include "compiler.h"
#include "parser.h"
#include "operators.h"
#include "source.h"
#include "sequence.h"
#include "generate.h"
#include "keywords.h"
#include "options.h"
#include "variable.h"

#include <opcodes.h>

#include <all_keywords.h>

using namespace primal;

std::shared_ptr<compiler> compiler::create()
{
    register_all_keywords();
    register_opcode_compilers();
    label::reset_counter();
    return std::make_shared<compiler>();
}

bool compiler::compile(const std::string &s)
{
    parser p;
    std::string trimmed = s;
    trimmed = util::strim(trimmed);
    m_src = source(trimmed);
    std::string last;
    auto seqs = p.parse(m_src, [&](std::string) {return false;}, last);

    // now compile the main namespace or whatever I will call it later
    for(const auto& seq : std::get<0>(seqs))
    {
        seq->compile(this);
    }

    // and the exit program byte
    compiled_code::instance(this).append(0xFF);

    auto fdecls = std::get<1>(seqs);
    for(const auto& seq : fdecls)
    {
        kw_fun* f = dynamic_cast<kw_fun*>(seq.get());
        if(!f)
        {
            throw "internal compiler error";
        }
        set_frame(f->function().get());
        f->compile(this);
    }

    // and now compile the functions in

    compiled_code::instance(this).finalize();

    return true;
}

std::vector<uint8_t> compiler::bytecode() const
{
    return compiled_code::instance(const_cast<compiler*>(this)).bytecode();
}

std::shared_ptr<generate> compiler::generator()
{
    return std::make_shared<generate>(this);
}

source &compiler::get_source()
{
    return m_src;
}

int compiler::next_varcount(fun* holder)
{
    if(m_varcounters.count(holder))
    {
        int retv = m_varcounters[holder];
        m_varcounters[holder] ++;
        return retv;
    }
    else
    {
        m_varcounters[holder] = 1;
        return 0;
    }
}

bool compiler::has_variable(const std::string &name)
{
    return variables.count(name) != 0;
}

std::shared_ptr<variable> compiler::get_variable(const std::string &name)
{
    if(!has_variable(name))
    {
        return std::shared_ptr<variable>();
    }
    return variables[name];
}

std::shared_ptr<variable> compiler::create_variable(const std::string &name)
{
    auto x = std::make_shared<variable>(this, name);
    variables[name] = x;
    return x;
}

void compiler::set_frame(fun *f)
{
    m_current_frame = f;
}

compiler::~compiler()
{
    if(options::instance().generate_assembly())
    {
        std::string s = options::instance().asm_stream().str();
        std::cout << "-------------------------------------------------" << std::endl << s << std::endl;
        options::instance().asm_stream().clear();
        options::instance().asm_stream().str(std::string());
    }

    compiled_code::instance(this).destroy();
    variable::reset();
}
