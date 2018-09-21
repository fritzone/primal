#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <sstream>

class source
{
public:
    source() = default;
    source(const char* src);
    source(const std::string& src);

    source& operator = (const source& oth)
    {
        m_ss << oth.m_ss.str();
        m_empty = oth.m_empty;
        return *this;
    }

    virtual ~source() = default;

    bool empty();
    std::string next();

private:
    std::stringstream m_ss;
    bool m_empty = false;
};

#endif
