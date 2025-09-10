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
#include <type_traits>
#include <numeric_decl.h> // For word_t

// --- Unified type for script values ---
using ScriptValue = std::variant<std::monostate, word_t, std::string>;
using ScriptArgs = std::vector<ScriptValue>;

// --- Helper for printing ScriptValue ---
inline std::ostream& operator<<(std::ostream& os, const ScriptValue& v) {
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

inline std::string get_value_type_name(const ScriptValue& v) {
    if (std::holds_alternative<std::monostate>(v)) return "void";
    if (std::holds_alternative<word_t>(v)) return "number";
    if (std::holds_alternative<std::string>(v)) return "string";
    return "unknown";
}


// --- The Core of the System: The Function Registry ---
class FunctionRegistry {
public:
    using GenericFunc = std::function<ScriptValue(const ScriptArgs&)>;

private:
    // --- Metaprogramming helper to deduce function signatures ---
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

    // --- Helpers for argument unpacking ---
    template <typename T>
    static T get_arg(const ScriptArgs& args, std::size_t index, const std::string& func_name) {
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

    template <typename... Args, std::size_t... Is>
    static std::tuple<Args...> unpack_args(const ScriptArgs& args, const std::string& func_name, std::index_sequence<Is...>) {
        return std::make_tuple(get_arg<Args>(args, Is, func_name)...);
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

        functions[name] = [func = std::move(func), name](const ScriptArgs& args) -> ScriptValue {
            if (traits::arity != args.size()) {
                throw std::runtime_error(
                    "Error calling '" + name + "': Expected " +
                    std::to_string(traits::arity) + " arguments, but got " +
                    std::to_string(args.size()) + "."
                    );
            }

            // Create a tuple of C++ arguments by unpacking the variant vector
            auto cpp_args_tuple = unpack_args_from_tuple<ArgsTuple>(args, name, std::make_index_sequence<traits::arity>{});

            if constexpr (std::is_void_v<Ret>) {
                std::apply(func, cpp_args_tuple);
                return std::monostate{};
            } else {
                return std::apply(func, cpp_args_tuple);
            }
        };
    }

    ScriptValue call(const std::string& name, const ScriptArgs& args) {
        auto it = functions.find(name);
        if (it == functions.end()) {
            throw std::runtime_error("Function not found: " + name);
        }
        return it->second(args);
    }

    static FunctionRegistry& instance() {
        static FunctionRegistry registry;
        return registry;
    }

private:
    FunctionRegistry() = default;
    std::map<std::string, GenericFunc> functions;

    // Helper to bridge the parameter-pack based unpack_args with the tuple from function_traits
    template<typename Tuple, typename... Args, std::size_t... Is>
    static Tuple unpack_args_from_tuple_impl(const ScriptArgs& args, const std::string& func_name, std::index_sequence<Is...>) {
        return std::make_tuple(get_arg<std::tuple_element_t<Is, Tuple>>(args, Is, func_name)...);
    }

    template<typename Tuple>
    static Tuple unpack_args_from_tuple(const ScriptArgs& args, const std::string& func_name, std::index_sequence_for<std::tuple_element_t<0, Tuple>> seq = {}) {
        return unpack_args_from_tuple_impl<Tuple>(args, func_name, std::make_index_sequence<std::tuple_size_v<Tuple>>{});
    }
};

#endif // FUNCTION_REGISTRY_H

