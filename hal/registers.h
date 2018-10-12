#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <types.h>

struct reg final
{
    reg()                   : m_reg_idx(0) {}
    explicit reg(uint8_t i) : m_reg_idx(i) {}

    reg& operator = (numeric_t v) { m_value = v; return *this; }

    bool operator == (numeric_t v) const { return m_value == v; }
    bool operator == (const reg& v) const { return m_value == v.m_value; }

    uint8_t idx() const {return m_reg_idx;}
    void set_idx(uint8_t i) { m_reg_idx = i; }

    numeric_t& value()      {return m_value;}
    numeric_t value() const {return m_value;}

    void set_value(numeric_t v) {m_value = v;}
    void set_value(const reg& ov) {m_value = ov.m_value;}


private:

    numeric_t m_value = 0;
    uint8_t m_reg_idx;
};

#endif
