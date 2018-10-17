#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>
#include <types.h>

#include <map>
#include <functional>

static constexpr std::array< std::pair<uint32_t, uint8_t> , 4> masks = {std::make_pair(0x000000FF, 0),
                                                                        std::make_pair(0x0000FF00, 8),
                                                                        std::make_pair(0x00FF0000, 16),
                                                                        std::make_pair(0xFF000000, 24)
};

/**
 * This defines a structure which can be assigend a value to. When you implement your opcodes
 * this will be the basic structure you will get as a parameter, the other derivations of it
 * represent the concrete objects the virtual machine has access to.
 *
 * The member @see set_value can be sued to set the value of this valued objects and the
 * member @see value can be used to query the value of it.
 **/
struct valued
{
    valued() = default;
    explicit valued(numeric_t v) : m_value(v) {}
    ~valued() = default;

    bool operator == (numeric_t v) const { return m_value == v; }

    valued& operator += (numeric_t v) { m_value += v; return *this; }
    valued& operator += (const valued& v) { m_value += v.value(); return *this; }

    valued& operator -= (numeric_t v) { m_value -= v; return *this; }
    valued& operator -= (const valued& v) { m_value -= v.value(); return *this; }

    valued& operator /= (numeric_t v) { m_value /= v; return *this; }
    valued& operator /= (const valued& v) { m_value /= v.value(); return *this; }

    valued& operator *= (numeric_t v) { m_value *= v; return *this; }
    valued& operator *= (const valued& v) { m_value *= v.value(); return *this; }

    valued& operator %= (numeric_t v) { m_value %= v; return *this; }
    valued& operator %= (const valued& v) { m_value %= v.value(); return *this; }

    valued& operator &= (numeric_t v) { m_value &= v; return *this; }
    valued& operator &= (const valued& v) { m_value &= v.value(); return *this; }

    valued& operator |= (numeric_t v) { m_value |= v; return *this; }
    valued& operator |= (const valued& v) { m_value |= v.value(); return *this; }

    valued& operator ^= (numeric_t v) { m_value ^= v; return *this; }
    valued& operator ^= (const valued& v) { m_value ^= v.value(); return *this; }

    valued& operator ++ () { m_value ++; return *this; }
    valued operator ++ (int) { valued ret(*this); ++(*this); return ret;}


    numeric_t& value()      {return m_value;}
    numeric_t value() const {return m_value;}

    virtual void set_value(numeric_t v) {m_value = v;}

    numeric_t m_value = 0;
};

/* This class represents a concrete register that can be found in the system */
struct reg final : public valued
{
    reg()                   : m_reg_idx(0) {}
    explicit reg(uint8_t i) : m_reg_idx(i) {}

    reg& operator = (numeric_t v) { m_value = v; return *this; }
    reg& operator = (const reg& ov) { m_value = ov.m_value; return *this; }

    bool operator == (const reg& v) const { return m_value == v.m_value; }
    using valued::operator==;

    void set_value(const reg& ov) {m_value = ov.m_value;}
    using valued::set_value;

    uint8_t idx() const {return m_reg_idx;}
    void set_idx(uint8_t i) { m_reg_idx = i; }

    uint8_t m_reg_idx;
};

/* This class represents a memory address that can be used in the system */
struct memaddress final : public valued
{
    memaddress() = default;
    explicit memaddress(numeric_t address, std::function<void(numeric_t, numeric_t)> setter) : m_address(address) {}
    void set_value(numeric_t v) override
    {
        m_setter(m_address, v);
    }

    numeric_t m_address = -1;
    std::function<void(numeric_t, numeric_t)> m_setter;
};

/* This class represents a sub byte of a register */
struct reg_subbyte : public valued
{
    reg_subbyte(reg* r, uint8_t bidx) : m_r(r), m_bidx(bidx) {}
    void set_value(numeric_t v) override
    {
        m_r->set_value( m_r->value() | (((v << masks[m_bidx].second) & masks[m_bidx].first) | !(masks[m_bidx].first)));
    }

    reg* m_r;
    uint8_t m_bidx;
};

/* This represents an immediate value */
struct immediate : public valued
{
    immediate() = default;
    explicit immediate(numeric_t p) { m_value = p; }

    void set_value(numeric_t v) override
    {
        throw "cannot do this";
    }
};

#endif
