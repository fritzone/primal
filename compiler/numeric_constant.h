#ifndef NUMERIC_CONSTANT_H
#define NUMERIC_CONSTANT_H


#include "sequence.h"

#include <string>

namespace primal
{
/* Simple class to represent a numeric_constant in the assembly output */
class numeric_constant : public sequence
{
public:

    explicit numeric_constant(source& src) : sequence(src) {}
    explicit numeric_constant(source& src, const std::string& s) : sequence(src), m_value(util::string_to_number<word_t>(s)) {}

    ~numeric_constant() override = default;

    prepared_type prepare(std::vector<token>& tokens) override;
    bool compile(compiler*) override;

    word_t value() const;
    void set_value(word_t newValue);

private:
    static uint32_t numeric_constant_counter;

private:
    word_t m_value = 0;

};

inline sequence::prepared_type numeric_constant::prepare(std::vector<token> &tokens)
{
    return {};
}

inline bool numeric_constant::compile(compiler *)
{
    return false;
}

inline word_t numeric_constant::value() const
{
    return m_value;
}

inline void numeric_constant::set_value(word_t newValue)
{
    m_value = newValue;
}

}


#endif // NUMERIC_CONSTANT_H
