#include "util.h"
#include "operators.h"

#include <algorithm>
#include <string>
#include <ctype.h>

bool util::is_identifier_char(char c) { return isalnum(c) || c == '_'; }
bool util::is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool util::is_integer(char c) { return (c >= '0') && (c <= '9'); }
bool util::is_operator(char c) { std::string s; s += c; return operators.count(s) > 0; }

void util::skip_whitepsace(size_t & i, const std::string & s) { while (i < s.length() && is_whitespace(s[i])) i++; }

std::string util::to_upper(const std::string & s)
{
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), ::toupper);
    return res;
}

bool util::is_comparison(const std::string &s)
{
    return s == "==" || s == "!=" || s == "<" || s == ">" || s == "<=" || s == ">=";
}

