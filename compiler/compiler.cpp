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
#include "function.h"

#include <opcodes.h>

#include <all_keywords.h>

#include <cstring>

#include <fstream>
#include <streambuf>
#include <iomanip>
#include <algorithm>

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
            throw std::runtime_error("internal compiler error, lost a function");
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

    // is this a parameter to current frame?
    if(m_current_frame)
    {
        if(m_current_frame->get_parameter(name) && variables[m_current_frame].count(name) == 0)
        {
            return create_variable(name);
        }
    }

    if(variables.count(m_current_frame) == 0)
    {
        return std::shared_ptr<variable>();
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

        // let's see if this is a parameter to a function or not
        auto&& p = m_current_frame->get_parameter(name);

        if(!p)
        {
            // This indicates the variable was not found in the static declaration list, nor a parameter
            // which would be an internal compiler error.
            throw syntax_error("Internal error: Attempted to instantiate undeclared variable '" + name + "'");
        }

        switch(p->type) {
            case primal::entity_type::ET_NUMERIC:
            {
                size = 0;
                break;
            }
            case primal::entity_type::ET_STRING:
            {
                size = 256;
                break;
            }

            default:
                throw syntax_error("Internal error: Attempted to instantiate a non typed variable '" + name + "'");
        }
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

std::vector<summary_pod> compiler::get_function_summaries() const {
    std::vector<summary_pod> summaries;
    const auto& functions = fun::get_functions();
    summaries.reserve(functions.size());

    for (const auto& pair : functions) {
        const auto& f = pair.second;
        summary_pod summary = {}; // Zero-initialize the struct

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
        std::cout << "-------------------------------------------------" << std::endl << std::endl;
    }

    print_function_summary();

    if (!m_interface_header_path.empty()) {
        try {
            generate_variable_interface();
            std::cout << "Successfully generated C++ interface header: " << m_interface_header_path << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error generating C++ interface header: " << e.what() << std::endl;
        }
    }

    compiled_code::instance(this).destroy();
    variable::reset();
    fun::reset();
}

void compiler::set_interface_header_path(const std::string& path, const std::string& script_name) {
    m_interface_header_path = path;
    m_script_name = script_name;
}

// Generates a C++-safe class name from a filename.
std::string script_name_to_class_name(const std::string& script_name) {
    std::string base_name = script_name;
    size_t last_slash = base_name.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        base_name = base_name.substr(last_slash + 1);
    }
    size_t first_dot = base_name.find('.');
    if (first_dot != std::string::npos) {
        base_name = base_name.substr(0, first_dot);
    }
    if (base_name.empty()) {
        return "PrimalScript";
    }
    // Capitalize first letter
    base_name[0] = static_cast<char>(toupper(base_name[0]));
    // Replace invalid characters
    std::replace_if(base_name.begin() + 1, base_name.end(), [](char c) {
        return !isalnum(c);
    }, '_');
    return base_name;
}

void compiler::generate_variable_interface() const {
    std::ofstream header_file(m_interface_header_path);
    if (!header_file.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + m_interface_header_path);
    }

    std::string class_name = script_name_to_class_name(m_script_name);

    header_file << "// Generated by the Primal Compiler for script '" << m_script_name << "' - Do not edit!\n";
    header_file << "#ifndef " << util::to_upper(class_name) << "_PRIM_H\n";
    header_file << "#define " << util::to_upper(class_name) << "_PRIM_H\n";

    header_file << "#define TARGET_ARCH " << TARGET_ARCH<< "\n";

    header_file << "#include <numeric_decl.h>\n";
    header_file << "#include <vm.h>\n";
    header_file << "#include <interface.h>\n";
    header_file << "#include <string>\n";
    header_file << "#include <stdexcept>\n";
    header_file << "#include <vector>\n";
    header_file << "#include <cstdint>\n\n";

    header_file << "class " << class_name << " {\n";
    header_file << "private:\n";
    header_file << "    std::shared_ptr<primal::vm> m_vm;\n";

    // Embed bytecode
    const auto bc = compiled_code::instance(const_cast<compiler*>(this)).bytecode();
    header_file << "    static const unsigned char s_bytecode[];\n";
    header_file << "    static const size_t s_bytecode_len = " << bc.size() << ";\n\n";

    header_file << "public:\n";
    header_file << "    " << class_name << "() {";
    header_file << "        m_vm = primal::vm::create();\n}\n\n";

    // FFI Registration
    header_file << "    template<typename Callable>\n";
    header_file << "    void register_function(const std::string& name, Callable func) {\n";
    header_file << "        m_vm->register_function(name, func);\n";
    header_file << "    }\n\n";

    // Run Method
    header_file << "    bool run() {\n";
    header_file << "        std::vector<uint8_t> bytecode_vec(s_bytecode, s_bytecode + s_bytecode_len);\n";
    header_file << "        return m_vm->run(bytecode_vec);\n";
    header_file << "    }\n\n";

    header_file << "    // --- Generated Accessors ---\n\n";

    word_t current_location = 0;
    for (const auto& var_info : variable::get_declarations()) {
        const auto& name = std::get<0>(var_info);
        const auto& type = std::get<1>(var_info);
        const auto& origin = std::get<2>(var_info);
        const auto& size = std::get<3>(var_info);

        if (origin == entity_origin::EO_VARIABLE && name.find(':') == std::string::npos) { // It's a global variable

            word_t base_address = current_location * word_size;

            if (size > 1) { // It's an array
                header_file << "    // Array: var " << to_string(type) << " " << name << "[" << size << "]\n";
                if (type == entity_type::ET_NUMERIC) {
                    header_file << "    word_t get_" << name << "(size_t index) {\n";
                    header_file << "        if (index >= " << size << ") { throw std::out_of_range(\"Index out of bounds for array '" << name << "'.\"); }\n";
                    header_file << "        return m_vm->get_mem(" << base_address << " + index * word_size);\n";
                    header_file << "    }\n\n";
                    header_file << "    void set_" << name << "(size_t index, word_t value) {\n";
                    header_file << "        if (index >= " << size << ") { throw std::out_of_range(\"Index out of bounds for array '" << name << "'.\"); }\n";
                    header_file << "        m_vm->set_mem(" << base_address << " + index * word_size, value);\n";
                    header_file << "    }\n\n";
                }
                // Note: Setters for string arrays are complex and omitted for now.
            } else { // It's a scalar variable
                header_file << "    // Scalar: var " << to_string(type) << " " << name << "\n";
                if (type == entity_type::ET_NUMERIC) {
                    header_file << "    word_t get_" << name << "() {\n";
                    header_file << "        return m_vm->get_mem(" << base_address << ");\n";
                    header_file << "    }\n\n";
                    header_file << "    void set_" << name << "(word_t value) {\n";
                    header_file << "        m_vm->set_mem(" << base_address << ", value);\n";
                    header_file << "    }\n\n";
                } else if (type == entity_type::ET_STRING) {
                    header_file << "    std::string get_" << name << "() {\n";
                    header_file << "        word_t string_addr = m_vm->get_mem(" << base_address << ");\n";
                    header_file << "        if (string_addr == 0) return \"\"; // Uninitialized string\n";
                    header_file << "        uint8_t len = m_vm->get_mem_byte(string_addr);\n";
                    header_file << "        std::string result;\n";
                    header_file << "        result.reserve(len);\n";
                    header_file << "        for (uint8_t i = 0; i < len; ++i) {\n";
                    header_file << "            result += static_cast<char>(m_vm->get_mem_byte(string_addr + 1 + i));\n";
                    header_file << "        }\n";
                    header_file << "        return result;\n";
                    header_file << "    }\n\n";
                    // Note: A setter for strings would require memory allocation inside the VM, which is complex.
                }
            }
            current_location += size;
        }
    }

    header_file << "}; // class " << class_name << "\n\n";

    // Write bytecode definition
    header_file << "const unsigned char " << class_name << "::s_bytecode[] = {\n    ";
    for (size_t i = 0; i < bc.size(); ++i) {
        header_file << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bc[i]);
        if (i != bc.size() - 1) {
            header_file << ", ";
        }
        if ((i + 1) % 16 == 0) {
            header_file << "\n    ";
        }
    }
    header_file << "\n};\n#endif\n";}
