#pragma once

#include <string>
#include <cctype>
#include <memory>
#include <algorithm>
#include <functional>
#include <map>

namespace util
{
    bool is_identifier_char(char c);
    bool is_whitespace(char c);
    bool is_integer(char c);
    bool is_operator(char c);
    bool is_comparison(const std::string& s);
    bool is_number(const std::string& s);

    void skip_whitepsace(size_t& i, const std::string& s);

    std::string to_upper(const std::string& s);
    template<class T, class... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    // trim from start
    static inline std::string &sltrim(std::string &s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
            return s;
    }

    // trim from end
    static inline std::string &srtrim(std::string &s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
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
}

