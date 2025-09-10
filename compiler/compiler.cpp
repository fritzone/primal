#include "compiler.h"
#include "parser.h"
#include "operators.h"
#include "source.h"
#include "sequence.h"
#include "generate.h"
#include "keywords.h"
#include "options.h"
#include "variable.h"
#include "hal.h"

#include <opcodes.h>

#include <all_keywords.h>

#include <cstring>

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
        options::instance().asm_stream() << std::setfill(' ') << std::right << std::setw(5) << std::dec
                                         << compiled_code::instance(this).location() + PRIMAL_HEADER_SIZE << ": (FF) EXIT" << std::endl;
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

std::shared_ptr<variable> primal::compiler::compiler::create_variable(const std::string &name)
{
    // This function should only be called if the variable is known to be declared.
    word_t size = variable::get_size(name);
    if (size == 0) {
        // This indicates the variable was not found in the static declaration list,
        // which would be an internal compiler error.
        throw syntax_error("Internal error: Attempted to instantiate undeclared variable '" + name + "'");
    }

    // Create the new variable object with the correct size.
    auto new_var = std::make_shared<variable>(this, name, size);

    // Store it in the compiler's instance map for the current scope.
    variables[m_current_frame][name] = new_var;

    return new_var;
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

std::map<std::string, std::shared_ptr<fun>> compiler::print_function_summary() {
    const auto& functions = fun::get_functions();

    std::cout << "\n--- Function Summary ---" << std::endl;
    for (const auto& pair : functions) {
        const auto& f = pair.second;

        // Print function address
        std::cout << "@0x" << std::hex << std::setw(4) << std::setfill('0')
                  << (f->get_address() + PRIMAL_HEADER_SIZE) << std::dec << ": ";

        std::cout << "fun " << f->name() << "(";

        bool first_param = true;
        entity_type last_type = entity_type::ET_UNKNOWN;

        for (const auto& param : f->get_parameters()) {
            if (param.type != last_type) {
                if (!first_param) {
                    std::cout << ", ";
                }
                std::cout << to_string(param.type) << " ";
                last_type = param.type;
            } else {
                if (!first_param) {
                    std::cout << " ";
                }
            }
            std::cout << param.name;
            first_param = false;
        }

        std::cout << ")";

        // Only print return type if it's not the default (numeric)
        if (f->get_return_type() == entity_type::ET_STRING || f->get_return_type() == entity_type::ET_NUMERIC) {
            std::cout << " " << to_string(f->get_return_type());
        }

        // is this extern?
        if (f->is_extern()) {
            std::cout << " EXTERN ";
        }

        std::cout << std::endl;
    }
    std::cout << "------------------------\n" << std::endl;

    return functions;
}

std::vector<primal::fun::summary_pod> compiler::get_function_summaries() const {
    std::vector<fun::summary_pod> summaries;
    const auto& functions = fun::get_functions();
    summaries.reserve(functions.size());

    for (const auto& pair : functions) {
        const auto& f = pair.second;
        fun::summary_pod summary = {}; // Zero-initialize the struct

        // Copy the function name, ensuring it is null-terminated
        strncpy(summary.name, f->name().c_str(), MAX_FUNCTION_NAME_LEN - 1);
        summary.name[MAX_FUNCTION_NAME_LEN - 1] = '\0'; // Ensure null termination

        summary.address = f->get_address();
        summary.is_extern = f->is_extern();
        summary.return_type = static_cast<uint8_t>(f->get_return_type());

        const auto& params = f->get_parameters();
        summary.parameter_count = static_cast<uint8_t>(params.size());

        if (summary.parameter_count > MAX_FUNCTION_PARAMS) {
            // Optional: Log a warning if a function has too many parameters
            std::cerr << "Warning: Function '" << f->name() << "' has more than "
                      << MAX_FUNCTION_PARAMS << " parameters. Truncating for summary." << std::endl;
            summary.parameter_count = MAX_FUNCTION_PARAMS;
        }

        // Copy the parameter types
        for (uint8_t i = 0; i < summary.parameter_count; ++i) {
            summary.parameter_types[i] = static_cast<uint8_t>(params[i].type);
        }

        summaries.push_back(summary);
    }
    return summaries;
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
