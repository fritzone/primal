#ifndef PRIMITIVE_OPCODE_H
#define PRIMITIVE_OPCODE_H

#include "numeric_decl.h"

#include <string>
#include <cstdint>
#include <functional>
#include <map>

namespace primal
{
namespace opcodes
{
/**
 * @brief Categories of opcodes grouped by their functionality.
 *
 * Each opcode belongs to a family that represents its general purpose,
 * such as arithmetic operations, memory manipulation, or control flow.
 */
enum class opcode_family : uint8_t
{
    /** Jump-related instructions (unconditional or conditional). */
    OF_JUMP = 0,

    /** Arithmetic operations (addition, subtraction, multiplication, etc.). */
    OF_ARITH = 1,

    /** Binary operations (bitwise AND, OR, XOR, shifts, etc.). */
    OF_BIN = 2,

    /** Stack manipulation (push, pop, etc.). */
    OF_STACK = 3,

    /** Comparison operations (equal, less than, etc.). */
    OF_COMP = 4,

    /** Memory operations (load, store, etc.). */
    OF_MEM = 5,

    /** Control flow instructions not related to direct jumps. */
    OF_CONTROLFLOW = 6,

    /** Other miscellaneous or unspecified instructions. */
    OF_OTHER = 7
};

/**
 * @brief Represents a single opcode in the Primal VM.
 *
 * This abstract base class defines the common interface that all opcodes
 * must implement, including their binary encoding, name, number of parameters,
 * and functional family.
 */
struct opcode
{
    /** Virtual destructor to allow proper cleanup of derived opcode types. */
    virtual ~opcode() = default;

    /**
     * @brief Returns the binary encoding value of this opcode.
     * @return The binary opcode value as an 8-bit integer.
     */
    virtual uint8_t bin() const = 0;

    /**
     * @brief Returns the textual name of this opcode.
     * @return The opcode name as a string.
     */
    virtual std::string name() const = 0;

    /**
     * @brief Returns the number of parameters this opcode requires.
     * @return The parameter count as a word_t.
     */
    virtual word_t paramcount() const = 0;

    /**
     * @brief Returns the functional family to which this opcode belongs.
     * @return The opcode family classification.
     */
    virtual opcode_family family() const = 0;
};
}
}

#endif // PRIMITIVE_OPCODE_H
