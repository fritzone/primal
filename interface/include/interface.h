#ifndef FUNCTION_REGISTRY_H
#define FUNCTION_REGISTRY_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <numeric_decl.h>
#include <vm.h>

namespace primal
{

/**
 * @brief Unified type representing a value in the scripting system.
 *
 * This variant type can hold:
 * - `std::monostate` (representing a void/empty value),
 * - `word_t` (a numeric value defined in `numeric_decl.h`),
 * - `std::string` (a string value).
 */
using script_value = std::variant<std::monostate, word_t, std::string>;

/**
 * @brief A list of script values used as function arguments.
 */
using script_args = std::vector<script_value>;

/**
 * @brief Stream output operator for @ref script_value.
 *
 * Prints a script value in human-readable form:
 * - `[void]` for `std::monostate`
 * - The number directly for `word_t`
 * - The string wrapped in quotes for `std::string`
 *
 * @param os Output stream.
 * @param v The script value to print.
 * @return Reference to the output stream.
 */
inline std::ostream& operator<<(std::ostream& os, const script_value& v) {
    std::visit([&os](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            os << "[void]";
        } else if constexpr (std::is_same_v<T, std::string>) {
            os << '"' << arg << '"';
        } else {
            os << arg;
        }
    }, v);
    return os;
}

/**
 * @brief Returns the type name of a script value.
 *
 * @param v The script value.
 * @return A string: `"void"`, `"number"`, `"string"`, or `"unknown"`.
 */
inline std::string get_value_type_name(const script_value& v) {
    if (std::holds_alternative<std::monostate>(v)) return "void";
    if (std::holds_alternative<word_t>(v)) return "number";
    if (std::holds_alternative<std::string>(v)) return "string";
    return "unknown";
}

/**
 * @brief A registry that allows registering and invoking C++ functions by name.
 *
 * The registry maps function names to generic wrappers that accept
 * script arguments (@ref script_args) and return a script value
 * (@ref script_value).
 *
 * Functions can be lambdas, free functions, or any callable that
 * has a well-defined signature. Arguments are automatically unpacked
 * from `script_args` and converted to the expected C++ types.
 */
class function_registry {
public:
    /**
     * @brief Type alias for a generic function wrapper.
     *
     * A function takes a vector of script arguments and returns a script value.
     */
    using generic_func = std::function<script_value(const script_args&)>;

private:
    /**
     * @brief Metaprogramming helper to deduce function signatures.
     *
     * Primary template: falls back to inspecting `operator()`
     * for lambdas and functors.
     *
     * @tparam T Callable type.
     */
    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    /**
     * @brief Function traits specialization for plain function signatures.
     *
     * @tparam Ret Return type.
     * @tparam Args Argument types.
     */
    template <typename Ret, typename... Args>
    struct function_traits<Ret(Args...)> {
        using return_type = Ret;              ///< Function return type.
        using args_tuple = std::tuple<Args...>; ///< Tuple of argument types.
        static constexpr size_t arity = sizeof...(Args); ///< Number of arguments.
    };

    /// Specialization for function pointers.
    template <typename Ret, typename... Args>
    struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

    /// Specialization for non-const member functions.
    template <typename C, typename Ret, typename... Args>
    struct function_traits<Ret(C::*)(Args...)> : function_traits<Ret(Args...)> {};

    /// Specialization for const member functions.
    template <typename C, typename Ret, typename... Args>
    struct function_traits<Ret(C::*)(Args...) const> : function_traits<Ret(Args...)> {};

    /**
     * @brief Helper to get a typed argument from a @ref script_args vector.
     *
     * @tparam T Expected argument type.
     * @param args Argument list.
     * @param index Argument index to extract.
     * @param func_name Name of the function being called (for error reporting).
     * @return Extracted argument of type `T`.
     *
     * @throws std::runtime_error If the index is out of range or type mismatched.
     */
    template <typename T>
    static T get_arg(const script_args& args, std::size_t index, const std::string& func_name) {
        if (index >= args.size()) {
            throw std::runtime_error("Argument index out of bounds for function '" + func_name + "'.");
        }
        try {
            return std::get<T>(args[index]);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error(
                "Type mismatch for argument " + std::to_string(index + 1) + " in function '" + func_name +
                "'. Got " + get_value_type_name(args[index]) + "."
                );
        }
    }

    /**
     * @brief Helper to unpack script arguments into a C++ tuple.
     *
     * @tparam Tuple The target tuple type.
     * @tparam Is Index sequence.
     * @param args Script arguments.
     * @param func_name Function name (for error reporting).
     * @return Tuple containing converted arguments.
     */
    template <typename Tuple, std::size_t... Is>
    static Tuple unpack_args_helper(const script_args& args, const std::string& func_name, std::index_sequence<Is...>) {
        return std::make_tuple(get_arg<std::tuple_element_t<Is, Tuple>>(args, Is, func_name)...);
    }

public:
    /**
     * @brief Register a function, lambda, or callable under a given name.
     *
     * @tparam F Callable type.
     * @param name Name to register the function under.
     * @param func Callable object.
     *
     * @throws std::runtime_error If argument count mismatch occurs at call time.
     */
    template<typename F>
    void add(const std::string& name, F func) {
        using traits = function_traits<std::decay_t<F>>;
        using ArgsTuple = typename traits::args_tuple;
        using Ret = typename traits::return_type;

        functions[name] = [func = std::move(func), name](const script_args& args) -> script_value {
            if (traits::arity != args.size()) {
                throw std::runtime_error(
                    "Error calling '" + name + "': Expected " +
                    std::to_string(traits::arity) + " arguments, but got " +
                    std::to_string(args.size()) + "."
                    );
            }

            // Convert arguments into a tuple
            auto cpp_args_tuple = unpack_args_helper<ArgsTuple>(args, name, std::make_index_sequence<traits::arity>{});

            if constexpr (std::is_void_v<Ret>) {
                std::apply(func, cpp_args_tuple);
                return std::monostate{};
            } else {
                return std::apply(func, cpp_args_tuple);
            }
        };
    }

    /**
     * @brief Call a registered function by name with arguments.
     *
     * @param name The function name.
     * @param args Arguments to pass.
     * @return The script value returned by the function.
     *
     * @throws std::runtime_error If the function is not found.
     */
    script_value call(const std::string& name, const script_args& args) {
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        return it->second(args);
    }

    /**
     * @brief Access the global singleton instance of the registry.
     *
     * @return Reference to the registry instance.
     */
    static function_registry& instance() {
        static function_registry registry;
        return registry;
    }

private:
    /// @brief Private constructor for singleton usage.
    function_registry() = default;

    /// @brief Map of function names to their generic wrappers.
    std::map<std::string, generic_func> functions;
};

} // namespace primal

#endif // FUNCTION_REGISTRY_H
