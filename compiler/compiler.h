#ifndef COMPILER_H
#define COMPILER_H

#include <memory>
#include <vector>
#include <map>

#include "source.h"
#include "numeric_decl.h"

namespace primal
{
class generate;
class variable;
class fun;

/**
 * @brief A plain, serializable structure to hold a function's summary.
 *
 * This structure can be directly serialized into bytecode
 * to form part of a symbol table. It contains metadata about a function,
 * including its name, address, parameter types, and return type.
 */
struct summary_pod {
    /// @brief The function name (up to 255 characters).
    char name[255] = {0};

    /// @brief Address of the function within the bytecode.
    word_t address;

    /// @brief Number of parameters this function accepts.
    uint8_t parameter_count {0};

    /// @brief Flag indicating whether the function is an external (C++) call.
    uint8_t is_extern {0};

    /// @brief Encoded return type of the function (see entity_type).
    uint8_t return_type {0};

    /// @brief Encoded parameter types of the function (see entity_type).
    uint8_t parameter_types[255] = {0};
};

/**
 * @brief Compiler wrapper for source-to-bytecode translation.
 *
 * This class manages the compilation process of source scripts
 * into bytecode. It maintains variables, function metadata,
 * code generation, and source preprocessing.
 */
class compiler
{
public:
    /**
     * @brief Factory function to create a compiler instance.
     *
     * This must be called before compilation.
     *
     * @return A shared pointer to a newly created compiler.
     */
    static std::shared_ptr<compiler> create();

    /// @brief Default constructor.
    compiler() = default;

    /// @brief Virtual destructor for polymorphic cleanup.
    virtual ~compiler();

    /**
     * @brief Compile the given instructions.
     *
     * @param s A string containing newline-separated instructions.
     * @return `true` if compilation succeeds, `false` otherwise.
     */
    bool compile(const std::string& s);

    /**
     * @brief Retrieve the bytecode of the latest compilation.
     *
     * @return Vector of raw bytecode bytes.
     */
    std::vector<uint8_t> bytecode() const;

    /**
     * @brief Get the code generator assigned to this compiler.
     *
     * @return Shared pointer to the generator.
     */
    std::shared_ptr<generate> generator();

    /**
     * @brief Access the source object used by this compiler.
     *
     * @return Reference to the compiler's source.
     */
    source& get_source();

    /**
     * @brief Get the next available variable index for a function.
     *
     * @param holder Function context to assign the variable to.
     * @param name Variable name.
     * @return Assigned variable index.
     */
    int next_varcount(fun* holder, const std::string &name);

    /**
     * @brief Get the last variable index used in a function.
     *
     * @param holder Function context.
     * @return Last assigned variable index.
     */
    int last_varcount(fun* holder);

    /**
     * @brief Check if a variable exists in the current scope.
     *
     * @param name Variable name.
     * @return `true` if variable exists, `false` otherwise.
     */
    bool has_variable(const std::string & name);

    /**
     * @brief Retrieve a variable by name.
     *
     * @param name Variable name.
     * @return Shared pointer to the variable, or `nullptr` if not found.
     */
    std::shared_ptr<variable> get_variable(const std::string & name);

    /**
     * @brief Create a new variable in the current scope.
     *
     * @param name Variable name.
     * @return Shared pointer to the newly created variable.
     */
    std::shared_ptr<variable> create_variable(const std::string& name);

    /**
     * @brief Set the current working function frame.
     *
     * @param f Function frame to set.
     */
    void set_frame(fun* f);

    /**
     * @brief Get the current working function frame.
     *
     * @return Pointer to the current function frame.
     */
    fun* frame() const { return m_current_frame; }

    /**
     * @brief Preprocess the source code.
     *
     * Applies preprocessor-like transformations to the script text.
     *
     * @param s Raw source code.
     * @return Preprocessed source code.
     */
    std::string preprocess(const std::string& s);

    /**
     * @brief Print a summary of all compiled functions.
     *
     * @return Map of function names to function objects.
     */
    std::map<std::string, std::shared_ptr<fun> > print_function_summary();

    /**
     * @brief Retrieve plain summaries of all compiled functions.
     *
     * @return Vector of @ref summary_pod structures.
     */
    std::vector<summary_pod> get_function_summaries() const;

    /**
     * @brief Set the interface header path for generated code.
     *
     * @param path Path to the header file.
     * @param script_name Name of the script being compiled.
     */
    void set_interface_header_path(const std::string& path, const std::string& script_name);

private:
    /**
     * @brief Generate the interface for variables.
     *
     * Used internally after compilation to expose variables
     * to the external interface.
     */
    void generate_variable_interface() const;

    /// @brief Source object containing the script.
    source m_src;

    /// @brief Map of function pointers to their variable counters.
    std::map<fun*, int> m_varcounters;

    /// @brief Map of variables per function context.
    std::map<fun*, std::map<std::string, std::shared_ptr<variable>>> variables;

    /// @brief Current working function frame.
    fun* m_current_frame = nullptr;

    /// @brief Path to the generated interface header.
    std::string m_interface_header_path;

    /// @brief Name of the script currently being compiled.
    std::string m_script_name;
};
}

#endif // COMPILER_H
