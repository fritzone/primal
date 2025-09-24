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
#include <utility> // For std::index_sequence

// Forward declaration for Primal numeric type
#include <numeric_decl.h>
#include <vm.h>

namespace primal
{

// Unified type for script values
using script_value = std::variant<std::monostate, word_t, std::string>;
using script_args = std::vector<script_value>;

// Helper for printing script_value
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

inline std::string get_value_type_name(const script_value& v) {
    if (std::holds_alternative<std::monostate>(v)) return "void";
    if (std::holds_alternative<word_t>(v)) return "number";
    if (std::holds_alternative<std::string>(v)) return "string";
    return "unknown";
}

// The Function Registry
class function_registry {
public:
    using generic_func = std::function<script_value(const script_args&)>;

private:
    // Metaprogramming helper to deduce function signatures from any callable
    template <typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    template <typename Ret, typename... Args>
    struct function_traits<Ret(Args...)> {
        using return_type = Ret;
        using args_tuple = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };

    template <typename Ret, typename... Args>
    struct function_traits<Ret(*)(Args...)> : function_traits<Ret(Args...)> {};

    template <typename C, typename Ret, typename... Args>
    struct function_traits<Ret(C::*)(Args...)> : function_traits<Ret(Args...)> {};

    template <typename C, typename Ret, typename... Args>
    struct function_traits<Ret(C::*)(Args...) const> : function_traits<Ret(Args...)> {};

    // Helper to get a typed argument from the script_args variant vector
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

    // A single, robust helper to unpack script arguments into a C++ tuple
    template <typename Tuple, std::size_t... Is>
    static Tuple unpack_args_helper(const script_args& args, const std::string& func_name, std::index_sequence<Is...>) {
        return std::make_tuple(get_arg<std::tuple_element_t<Is, Tuple>>(args, Is, func_name)...);
    }


public:
    /**
     * @brief Registers a C++ function, lambda, or any callable object.
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

            // Create a tuple of C++ arguments by unpacking the variant vector
            auto cpp_args_tuple = unpack_args_helper<ArgsTuple>(args, name, std::make_index_sequence<traits::arity>{});

            if constexpr (std::is_void_v<Ret>) {
                std::apply(func, cpp_args_tuple);
                return std::monostate{};
            } else {
                return std::apply(func, cpp_args_tuple);
            }
        };
    }

    script_value call(const std::string& name, const script_args& args) {
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        return it->second(args);
    }

    static function_registry& instance() {
        static function_registry registry;
        return registry;
    }

private:
    function_registry() = default;
    std::map<std::string, generic_func> functions;
};

}

#endif // FUNCTION_REGISTRY_H
