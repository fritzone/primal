#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <sstream>

namespace primal
{
/* This class represents a source object the compiler uses to hold code in */
    class source
    {
    public:
        /*Constructors*/
        source() = default;
        explicit source(const char* src);
        explicit source(const std::string& src);

        /*Assignment OP*/
        source& operator = (const source& oth)
        {
            m_ss << oth.m_ss.str();
            m_empty = oth.m_empty;
            return *this;
        }

        /* Destructor */
        ~source() = default;

        /* Tells us if we have reached the end of the source */
        bool empty();

        /* Gets the next string from the source code */
        std::string next();

    private:

        std::stringstream m_ss;
        bool m_empty = false;
    };

}

#endif
