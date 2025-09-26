#ifndef REGISTERS_H
#define REGISTERS_H

#include "numeric_decl.h"
#include "type_destination_decl.h"

#include <stdint.h>

#include <map>
#include <functional>
#include <array>
#include <stdexcept>

namespace primal
{
#if TARGET_ARCH == 32
/**
 * @brief Bit masks and shift offsets for accessing individual bytes
 *        within a 32-bit register.
 */
static constexpr std::array<std::pair<uint32_t, uint8_t>, 4> masks = {
    std::make_pair(0x000000FF, 0),
    std::make_pair(0x0000FF00, 8),
    std::make_pair(0x00FF0000, 16),
    std::make_pair(0xFF000000, 24)
};
#else
/**
 * @brief Bit masks and shift offsets for accessing individual bytes
 *        within a 64-bit register.
 */
static constexpr std::array<std::pair<uint64_t, uint8_t>, 8> masks = {
    std::make_pair(0x00000000000000FF, 0),
    std::make_pair(0x000000000000FF00, 8),
    std::make_pair(0x0000000000FF0000, 16),
    std::make_pair(0x00000000FF000000, 24),
    std::make_pair(0x000000FF00000000, 32),
    std::make_pair(0x0000FF0000000000, 40),
    std::make_pair(0x00FF000000000000, 48),
    std::make_pair(0xFF00000000000000, 56)
};
#endif

/**
 * @brief Represents a generic value container in the VM.
 *
 * This is the base structure for VM objects that can hold and manipulate values.
 * It supports arithmetic, bitwise, and comparison operators, making it suitable
 * for use by registers, memory addresses, and immediate values.
 */
struct valued
{
    valued() = default;
    explicit valued(word_t v) : m_value(v) {}
    ~valued() = default;

    valued(const valued& o) { set_value(o.value()); }

    valued& operator=(word_t v) { set_value(v); return *this; }
    valued& operator=(const valued& o) { set_value(o.value()); return *this; }

    // Comparison operators
    bool operator==(word_t v) const { return value() == v; }
    bool operator==(const valued& v) const { return value() == v.value(); }
    bool operator!=(word_t v) const { return value() != v; }
    bool operator!=(const valued& v) const { return value() != v.value(); }
    bool operator>=(word_t v) const { return value() >= v; }
    bool operator>=(const valued& v) const { return value() >= v.value(); }
    bool operator<=(word_t v) const { return value() <= v; }
    bool operator<=(const valued& v) const { return value() <= v.value(); }
    bool operator>(word_t v) const { return value() > v; }
    bool operator>(const valued& v) const { return value() > v.value(); }
    bool operator<(word_t v) const { return value() < v; }
    bool operator<(const valued& v) const { return value() < v.value(); }

    // Arithmetic and bitwise operators
    valued& operator+=(word_t v) { set_value(value() + v); return *this; }
    valued& operator+=(const valued& v) { set_value(value() + v.value()); return *this; }
    valued& operator-=(word_t v) { set_value(value() - v); return *this; }
    valued& operator-=(const valued& v) { set_value(value() - v.value()); return *this; }
    valued& operator/=(word_t v) { set_value(value() / v); return *this; }
    valued& operator/=(const valued& v) { set_value(value() / v.value()); return *this; }
    valued& operator*=(word_t v) { set_value(value() * v); return *this; }
    valued& operator*=(const valued& v) { set_value(value() * v.value()); return *this; }
    valued& operator%=(word_t v) { set_value(value() % v); return *this; }
    valued& operator%=(const valued& v) { set_value(value() % v.value()); return *this; }
    valued& operator&=(word_t v) { set_value(value() & v); return *this; }
    valued& operator&=(const valued& v) { set_value(value() & v.value()); return *this; }
    valued& operator|=(word_t v) { set_value(value() | v); return *this; }
    valued& operator|=(const valued& v) { set_value(value() | v.value()); return *this; }
    valued& operator^=(word_t v) { set_value(value() ^ v); return *this; }
    valued& operator^=(const valued& v) { set_value(value() ^ v.value()); return *this; }

    /** @return The current stored value. */
    virtual word_t value() const { return m_value; }

    /** @brief Updates the stored value. */
    virtual void set_value(word_t v) { m_value = v; }

    /** @return The type classification of this valued object. */
    virtual type_destination get_type() const = 0;

    /** @return A debug string representation of this object. */
    virtual std::string debug() const = 0;

    /** The stored value. */
    word_t m_value = 0;
};

/**
 * @brief Represents a physical register in the VM.
 *
 * Each register has an index and supports standard arithmetic, assignment,
 * and increment/decrement operations.
 */
struct reg final : public valued
{
    reg() : m_reg_idx(0) {}
    explicit reg(uint8_t i) : m_reg_idx(i) {}
    reg(const reg& o) : valued(o.value()), m_reg_idx(o.m_reg_idx) {}

    reg& operator=(word_t v) { m_value = v; return *this; }
    reg& operator=(const reg& ov) { m_value = ov.m_value; return *this; }

    bool operator==(const reg& v) const { return m_value == v.m_value; }
    using valued::operator==;

    /** Post-increment operator. */
    reg operator++(int) { reg temp = *this; m_value++; return temp; }

    /** Post-decrement operator. */
    reg operator--(int) { reg temp = *this; m_value--; return temp; }

    /** Conversion to word_t. */
    operator word_t() const { return m_value; }

    void set_value(const reg& ov) { m_value = ov.m_value; }
    using valued::set_value;

    /** @return The register index. */
    uint8_t idx() const { return m_reg_idx; }

    /** @brief Updates the register index. */
    void set_idx(uint8_t i) { m_reg_idx = i; }

    type_destination get_type() const override { return type_destination::TYPE_MOD_REG; }
    std::string debug() const override;

    /** The index of the register. */
    uint8_t m_reg_idx;
};

/**
 * @brief Represents a memory address reference in the VM.
 *
 * The memory address is accessed using provided getter and setter functions.
 */
struct memaddress final : public valued
{
    memaddress() = default;
    explicit memaddress(word_t address,
                        std::function<void(word_t, word_t)> setter,
                        std::function<word_t(word_t)> getter)
        : m_address(address), m_getter(std::move(getter)), m_setter(std::move(setter)) {}

    void set_value(word_t v) override { m_setter(m_address, v); }
    word_t value() const override { return m_getter(m_address); }

    type_destination get_type() const override { return type_destination::TYPE_MOD_MEM_REG_IDX; }
    std::string debug() const override;

    word_t m_address = -1;
    std::function<word_t(word_t)> m_getter;
    std::function<void(word_t, word_t)> m_setter;
};

/**
 * @brief Represents a byte reference at a memory address.
 *
 * Provides access to individual bytes using byte-level getter and setter functions.
 */
struct memaddress_byte_ref final : public valued
{
    memaddress_byte_ref() = default;
    explicit memaddress_byte_ref(word_t address,
                                 std::function<void(word_t, uint8_t)> setter,
                                 std::function<uint8_t(word_t)> getter)
        : m_address(address), m_getter(std::move(getter)), m_setter(std::move(setter)) {}

    void set_value(word_t v) override { m_setter(m_address, static_cast<uint8_t>(v)); }
    word_t value() const override { return m_getter(m_address); }

    type_destination get_type() const override { return type_destination::TYPE_MOD_MEM_REG_BYTE; }
    std::string debug() const override;

    word_t m_address = -1;
    std::function<uint8_t(word_t)> m_getter;
    std::function<void(word_t, uint8_t)> m_setter;
};

/**
 * @brief Represents a sub-byte reference of a register.
 *
 * Allows addressing specific bytes within a register using masks and shifts.
 */
struct reg_subbyte : public valued
{
    reg_subbyte(reg* r, uint8_t bidx) : m_r(r), m_bidx(bidx) {}

    void set_value(word_t v) override
    {
        m_r->set_value(((m_r->value()) |
                        (((v << masks[m_bidx].second) & masks[m_bidx].first) |
                         !(masks[m_bidx].first))));
    }

    type_destination get_type() const override { return type_destination::TYPE_MOD_REG_BYTE; }
    std::string debug() const override;

    reg* m_r;
    uint8_t m_bidx;
};

/**
 * @brief Represents an immediate (literal) value in the VM.
 *
 * Immediate values cannot be reassigned and are treated as constants.
 */
struct immediate : public valued
{
    immediate() = default;
    explicit immediate(word_t p) { m_value = p; }

    void set_value(word_t) override
    {
        throw std::runtime_error("invalid binary: cannot assign to a numeric value");
    }

    type_destination get_type() const override { return type_destination::TYPE_MOD_IMM; }
    std::string debug() const override;
};

/**
 * @brief Represents an immediate byte (literal 8-bit value).
 *
 * Similar to @ref immediate but restricted to byte-sized constants.
 */
struct immediate_byte : public valued
{
    immediate_byte() = default;
    explicit immediate_byte(word_t p) { m_value = p; }

    void set_value(word_t) override
    {
        throw std::runtime_error("invalid binary: cannot assign to a numeric value");
    }

    type_destination get_type() const override { return type_destination::TYPE_MOD_IMM_BYTE; }
    std::string debug() const override;
};

} // namespace primal

#endif // REGISTERS_H
