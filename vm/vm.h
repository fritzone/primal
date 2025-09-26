#ifndef VM_H
#define VM_H

#include <hal.h>
#include <opcode.h>
#include "numeric_decl.h"
#include <registers.h>
#include <interface.h>
#include "loaded_function.h"

#include <memory>
#include <vector>
#include <iostream>
#include <functional>

#define VM_DEBUG 1

/**
 * @brief Describes whether opcode debugging occurs before or after execution.
 */
enum class OpcodeDebugState
{
    VM_DEBUG_BEFORE = 1, /**< Debug state before opcode execution */
    VM_DEBUG_AFTER  = 2  /**< Debug state after opcode execution */
};

namespace primal
{

struct vm_impl;

/**
 * @brief Virtual Machine class responsible for executing compiled bytecode.
 *
 * This class manages the memory, registers, stack, and execution of instructions.
 * It also provides helper functions for memory access, stack operations, and
 * interrupt handling.
 */
class vm final
{
public:

    /**
     * @brief Create a new virtual machine instance.
     *
     * @return A shared pointer to a newly created VM instance.
     */
    static std::shared_ptr<vm> create();

    /**
     * @brief Default constructor.
     *
     * Initializes the virtual machine.
     */
    vm();

    /**
     * @brief Execute the compiled bytecode of an application.
     *
     * @param app The compiled bytecode to run.
     * @return True if execution completed normally.
     * @throws primal::vm_panic if invalid bytecode is detected.
     */
    bool run(const std::vector<uint8_t>& app);

    /**
     * @brief Accessor for the instruction pointer (IP) of the VM.
     *
     * Provides read/write access.
     *
     * @return Reference to the IP.
     */
    word_t& ip();

    /**
     * @brief Read-only accessor for the instruction pointer.
     *
     * @return The current IP value.
     */
    word_t ip() const;

    /**
     * @brief Write a word to VM memory.
     *
     * @param address Address to write to.
     * @param new_value The word value to write.
     * @throws primal::vm_panic if the address is invalid.
     */
    void set_mem(word_t address, word_t new_value);

    /**
     * @brief Read a word from VM memory.
     *
     * @param address Address to read from.
     * @return The word value at the given address.
     * @throws primal::vm_panic if the address is invalid.
     */
    word_t get_mem(word_t address);

    /**
     * @brief Write a byte to VM memory.
     *
     * @param address Address to write to.
     * @param b The byte value to write.
     * @throws primal::vm_panic if the address is invalid.
     */
    void set_mem_byte(word_t address, uint8_t b);

    /**
     * @brief Read a byte from VM memory.
     *
     * @param address Address to read from.
     * @return The byte value at the given address.
     * @throws primal::vm_panic if the address is invalid.
     */
    uint8_t get_mem_byte(word_t address);

    /**
     * @brief Access a register for read/write.
     *
     * @param i Index of the register.
     * @return Reference to the register.
     */
    reg& r(uint8_t i);

    /**
     * @brief Access a register for read-only.
     *
     * @param i Index of the register.
     * @return Const reference to the register.
     */
    const reg& r(uint8_t i) const;

    /**
     * @brief Copy a block of memory inside the VM.
     *
     * @param dest Destination address.
     * @param src Source address.
     * @param cnt Number of bytes to copy.
     * @return True if addresses are valid, false otherwise.
     */
    bool copy(word_t dest, word_t src, word_t cnt);

    /**
     * @brief Push a value onto the VM stack.
     *
     * @param v Pointer to a valued object.
     * @return True if the value was successfully pushed.
     */
    bool push(const valued* v);

    /**
     * @brief Push a numeric value onto the VM stack.
     *
     * @param v The word value to push.
     * @return True if the value was successfully pushed.
     */
    bool push(const word_t v);

    /**
     * @brief Pop a value from the VM stack.
     *
     * @return The word value popped from the stack.
     * @throws primal::vm_panic if the stack is empty.
     */
    word_t pop();

    /**
     * @brief Handle an exceptional VM state.
     *
     * Performs memory dump and terminates execution.
     *
     * @param reason Description of the error.
     */
    [[noreturn]] void panic(const char* reason);

    /**
     * @brief Fetch the next object from memory for opcode operations.
     *
     * @return Pointer to the valued object.
     */
    valued* fetch();

    /**
     * @brief Get the VM's last operation flag.
     *
     * @return The flag value.
     */
    word_t flag() const;

    /**
     * @brief Set the VM's flag value.
     *
     * @param v New flag value.
     */
    void set_flag(word_t v);

    /**
     * @brief Execute a jump in the VM memory.
     *
     * @param v Address to jump to.
     * @return True if the address is within valid memory range.
     */
    bool jump(word_t v);

    /**
     * @brief Trigger a VM interrupt.
     *
     * @param i Interrupt number.
     * @return True if interrupt executed successfully.
     */
    bool interrupt(word_t i);

    /**
     * @brief Check if a memory address is valid.
     *
     * @param addr The address to check.
     * @return True if the address is valid.
     */
    bool address_is_valid(word_t addr);

    /**
     * @brief Output opcode debugging information.
     *
     * @param o The opcode to debug.
     * @param ods The debug state.
     */
    void debug(primal::opcodes::opcode&& o, OpcodeDebugState ods);

    /**
     * @brief Retrieve the implementation pointer.
     *
     * @return Shared pointer to the internal implementation.
     */
    std::shared_ptr<vm_impl> get_impl() const;

    /**
     * @brief Register a host function to the VM.
     *
     * @tparam C Function type.
     * @param name Name of the function.
     * @param func Callable object.
     */
    template<typename C>
    void register_function(std::string name, C func)
    {
        function_registry::instance().add(name, func);
    }

    /**
     * @brief Enable or disable debug mode.
     *
     * @param newDebug New debug state.
     */
    void set_debug(bool newDebug);

#ifdef TICKS
    /**
     * @brief Set the execution speed of the VM.
     *
     * @param hertz Frequency in Hertz.
     */
    void set_speed(uint64_t hertz);
#endif

    /**
     * @brief Retrieve all loaded functions from the VM.
     *
     * @return Vector of loaded_function objects.
     */
    std::vector<loaded_function> functions() const;

private:

    std::shared_ptr<vm_impl> m_impl; /**< Internal implementation pointer */
    bool m_debug = false; /**< Debug flag */
    std::vector<loaded_function> m_functions; /**< Cached function table */
};

} // namespace primal

#endif // VM_H
