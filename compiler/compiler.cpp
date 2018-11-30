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

#include <fstream>
#include <streambuf>
#include <iomanip>

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
    std::string preprocessed = preprocess(s);

    std::string trimmed = preprocessed;
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
    if(options::instance().generate_assembly())
    {
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(5) << std::dec << compiled_code::instance(this).location() + 16 << ": (FF) EXIT" << std::endl;
    }

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

int compiler::next_varcount(fun* holder, const std::string& name)
{
    if(holder)
    {
        parameter* p = holder->get_parameter(name);
        if(p)
        {
            int idx = holder->get_parameter_index(p);
            if(idx > 0)
            {
                return - (idx + 2 );   // if it is a parameter substract: BP - 2 * num-t_size - parcount * num-t_size
            }
        }
    }

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

int compiler::last_varcount(fun *holder)
{
    if(m_varcounters.count(holder))
    {
        int retv = m_varcounters[holder];
        return retv;
    }
    return 0;
}

bool compiler::has_variable(const std::string &name)
{
    if(variables.count(m_current_frame) == 0)
    {
        return false;
    }

    // is this a paremeter to current frame?
    if(m_current_frame)
    {
        if(m_current_frame->get_parameter(name))
        {
            return true;
        }
    }

    if (variables[m_current_frame].count(name) == 0)
    {
        // is it a global variable?
        if(variables.count(nullptr) == 0)
        {
            return false;
        }
        return (variables[nullptr].count(name) != 0);
    }
    else
    {
        return true;
    }
}

std::shared_ptr<variable> compiler::get_variable(const std::string &name)
{
    if(variables.count(m_current_frame) == 0)
    {
        return std::shared_ptr<variable>();
    }

    // is this a paremeter to current frame?
    if(m_current_frame)
    {
        if(m_current_frame->get_parameter(name) && variables[m_current_frame].count(name) == 0)
        {
            return create_variable(name);
        }
    }

    if(variables[m_current_frame].count(name) == 0)
    {
        if(variables.count(nullptr) == 0)
        {
            return std::shared_ptr<variable>();
        }
        if(variables[nullptr].count(name) == 0)
        {
            return std::shared_ptr<variable>();
        }
        return variables[nullptr][name];
    }
    return variables[m_current_frame][name];
}

std::shared_ptr<variable> compiler::create_variable(const std::string &name)
{
    auto x = std::make_shared<variable>(this, name);
    // this will create BOTH variables[m_current_frame] and also the corresponding name if there is none
    variables[m_current_frame][name] = x;
    return x;
}

void compiler::set_frame(fun *f)
{
    m_current_frame = f;
}

std::string compiler::preprocess(const std::string& s)
{
    std::string result;

    std::istringstream f(s);
    std::string line;
    while (std::getline(f, line))
    {
        util::strim(line);
        if(line.find("import") == 0)
        {
            std::string package = line.substr(6);
            util::strim(package);
            package += ".prim";

            std::ifstream pf(package);
            std::string app((std::istreambuf_iterator<char>(pf)),
                             std::istreambuf_iterator<char>());

            std::string preprocessed_package = preprocess(app);
            result += preprocessed_package;

        }
        else
        {
            if(!line.empty() && line[0] != '#')
            {
                result += line + "\n";
            }
        }
    }

    return result;
}

compiler::~compiler()
{
    if(options::instance().generate_assembly())
    {
        //std::string s = options::instance().asm_stream().str();
        std::cout << "-------------------------------------------------" << std::endl << std::endl;
        ///options::instance().asm_stream().clear();
        //options::instance().asm_stream().str(std::string());
    }

    compiled_code::instance(this).destroy();
    variable::reset();
    fun::reset();
}
