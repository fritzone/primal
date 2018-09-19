#ifndef SOURCE_H
#define SOURCE_H

#include <string>
#include <sstream>

class source
{
public:
    source(const char* src);
    source(const std::string& src);

    virtual ~source() = default;

    bool empty();
    std::string next();

private:
    std::stringstream m_ss;
    bool m_empty = false;
};

#endif
