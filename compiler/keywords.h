#ifndef KEYWORDS_H
#define KEYWORDS_H

#include "source.h"
#include "sequence.h"

#include <string>
#include <map>
#include <functional>
#include <memory>
#include <iostream>

namespace primate
{
    class keyword
    {
    public:
        static std::map<std::string, std::function<std::shared_ptr<sequence>(source&)>> store;


    public:

        keyword() = default;
        virtual ~keyword() = default;
        virtual std::string name() = 0;
    };

}

/* This method will create a lambda for the specific type to create the required keyword */
template<class T> bool register_keyword()
{
    auto l = [&](primate::source& s) {return std::shared_ptr<T>(new T(s));};
    primate::keyword::store[T::N] = l;
    return true;
}

#define CONCAT_IMPL( x, y ) x##y
#define MACRO_CONCAT( x, y ) CONCAT_IMPL( x, y )
#define REGISTER_KEYWORD(w) static bool MACRO_CONCAT( b, __COUNTER__ ) = register_keyword<w>();

#endif
