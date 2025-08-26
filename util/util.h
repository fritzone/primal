#pragma once

#include <string>
#include <cctype>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>
#include <vector>
#include <sstream>
#include <type_traits>


namespace primal{
class token;
}

namespace util
{
    bool is_identifier_char(char c);
    bool is_whitespace(char c);
    bool is_integer(char c);
    bool is_operator(char c);
    bool is_comparison(const std::string& s);
    bool is_number(const std::string& s);
    bool is_logical_op(const std::string &s);

    void skip_whitepsace(size_t& i, const std::string& s);

    std::string to_upper(const std::string& s);
    template<class T, class... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // trim from start (left)
    static inline std::string& sltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                        [](unsigned char ch) { return !std::isspace(ch); }));
        return s;
    }

    // trim from end (right)
    static inline std::string& srtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                             [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        return s;
    }

    // trim from both ends
    static inline std::string strim(std::string &s) {
            return sltrim(srtrim(s));
    }


    template <typename T, typename U>
    class create_map
    {
    private:
        std::map<T, U> m_map;
    public:
        create_map(T key, U val)
        {
            m_map.emplace(std::move(key), std::move(val));
        }

        create_map&& operator()(T key, U val) &&
        {
            m_map.emplace(std::move(key), std::move(val));
            return std::move(*this);
        }

        operator std::map<T, U>() &&
        {
            return std::move(m_map);
        }
    };

    template<typename E>
    constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
    {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }

    /** Simple class reused for the purpose of this app.
     * @author Iain Hull (http://iainhull.github.io/)
     */
    class InputParser{
        public:
            InputParser (int &argc, char **argv){
                for (int i=1; i < argc; ++i)
                    this->tokens.push_back(std::string(argv[i]));
            }
            /// @author iain
            const std::string& getCmdOption(const std::string &option) const;
            /// @author iain
            bool cmdOptionExists(const std::string &option) const;
        private:
            std::vector <std::string> tokens;
    };

    template <typename T>
    std::string to_string(const T& value) {
        if constexpr (std::is_arithmetic_v<T>) {
            // For numbers, delegate to std::to_string (avoids iostream overhead)
            return std::to_string(value);
        } else {
            std::stringstream oss;
            oss << value;
            return oss.str();
        }
    }

    template <typename T>
    T string_to_number(const std::string& str) {
        std::istringstream iss(str);
        T num{};
        iss >> num;

        if (iss.fail() || !iss.eof()) {
            throw std::invalid_argument("Invalid numeric string: " + str);
        }
        return num;
    }
}

