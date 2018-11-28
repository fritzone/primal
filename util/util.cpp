#include "util.h"
#include "operators.h"

#include <algorithm>
#include <string>
#include <ctype.h>

bool util::is_identifier_char(char c) { return isalnum(c) || c == '_'; }
bool util::is_whitespace(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool util::is_integer(char c) { return (c >= '0') && (c <= '9'); }
bool util::is_operator(char c) { std::string s; s += c; return primal::operators.count(s) > 0; }

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

bool util::is_logical_op(const std::string &s)
{
    return s == "and" || s == "or";
}

bool util::is_number(const std::string &s)
{
    return !s.empty() && std::find_if(s.begin(),
                                      s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}


const std::string &util::InputParser::getCmdOption(const std::string &option) const{
    std::vector<std::string>::const_iterator itr;
    itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
    if (itr != this->tokens.end() && ++itr != this->tokens.end()){
        return *itr;
    }
    static const std::string empty_string("");
    return empty_string;
}

bool util::InputParser::cmdOptionExists(const std::string &option) const{
    return std::find(this->tokens.begin(), this->tokens.end(), option)
            != this->tokens.end();
}
