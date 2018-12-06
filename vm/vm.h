#ifndef VM_H
#define VM_H

#include <hal.h>
#include <numeric_decl.h>
#include <registers.h>

#include <memory>
#include <vector>

namespace primal
{

    struct vm_impl;

    /**
     * @brief The vm class is responsible for running compiled bytecode
     */
    class vm final
    {

    public:

        /**
         * @brief create Will create a new virtual machine you can use to run code
         * @return A newly created virtual machine (wrapped in a std::shared_ptr)
         */
        static std::shared_ptr<vm> create();

    public:

        /**
         * @brief vm Constructor, initializes the object
         */
        vm();

        /**
         * @brief run will run the given compiled application. You can use this together with the result
         *            of the compiler
         *
         * You can use this method to run the compiled bytecode. The method will return true if it managed
         * to run fully the application which exited normally. Every other situation will be rewarded by
         * throwing a primal::vm_panic exception.
         *
         * @throws primal::vm_panic in case of invalid bytecode
         * @param app the application to run
         * @return the method returns only true or it throws an exception
         */
        bool run(const std::vector<uint8_t>& app);

        /**
         * @brief ip Access function for the IP of the VM.
         *
         * This gives R/W acces, so You can use this to modify its value.
         *
         * @return the IP of the application
         */
        word_t& ip();

        /**
         * @brief ip Access function for the IP of the VM.
         *
         * This gives R/O acces, so You cannot use this to modify its value.
         *
         * @return the IP of the application
         */
        word_t ip() const;

        /**
         * @brief set_mem will set the memory at the given addres to the new value
         *
         * You can use this method to set a word in the memory of the virtual machine to the specified value. If
         * the address is invalid (ie: less tan 0 or greater than the allocated memory segments' size) the
         * application will throw a primal::vm_panic exception
         *
         * @throws primal::vm_panic in case of invalid address
         * @param address the address where the word will be placed
         * @param new_value the value that will be placed in the memory
         */
        void set_mem(word_t address, word_t new_value);

        /**
         * @brief get_mem can be used to fetch a word from the memory of the virtual machine
         *
         * You can use this method to get a word from the memory of the virtual machine. If
         * the address is invalid (ie: less tan 0 or greater than the allocated memory segments' size) the
         * application will throw a primal::vm_panic exception
         *
         * @throws primal::vm_panic in case of invalid address
         * @param address from where to get the word from the memory
         * @return the value as read from the memory
         */
        word_t get_mem(word_t address);

        /**
         * @brief set_mem_byte will set the byte in the memory at the given addres to the new byte value
         *
         * You can use this method to set a byte in the memory of the virtual machine to the specified value. If
         * the address is invalid (ie: less tan 0 or greater than the allocated memory segments' size) the
         * application will throw a primal::vm_panic exception
         *
         * @throws primal::vm_panic in case of invalid address
         * @param address the address where we want to change the byte
         * @param new_value the new value that will be written to the address
         */
        void set_mem_byte(word_t address, uint8_t b);
        /**
         * @brief get_mem can be used to fetch a word from the memory of the virtual machine
         *
         * You can use this method to get a word from the memory of the virtual machine. If
         * the address is invalid (ie: less tan 0 or greater than the allocated memory segments' size) the
         * application will throw a primal::vm_panic exception
         *
         * @throws primal::vm_panic in case of invalid address
         * @param address from where to get the word from the memory
         * @return the value as read from the memory
         */
        uint8_t get_mem_byte(word_t address);

        /**
         * @brief r is access function for the the given register.
         *
         * You can use this to modify the value of the register
         *
         * @param i the index of the register
         * @return a \c reg object to access the value of the register
         */
        reg& r(uint8_t i);

        /**
         * @brief r is access function for the the given register.
         *
         * You can use this to read the value of the register
         *
         * @param i the index of the register
         * @return a \c reg object to read the value of the register
         */
        const reg& r(uint8_t i) const;

        /**
         * @brief copy Copies a buffer in the memory area of the VM
         *
         * @param dest The destination address
         * @param src The source address
         * @param cnt The number of bytes to oppy
         *
         * @return True or false, depending if the address were correct or not
         */
        bool copy(word_t dest, word_t src, word_t cnt);

        /**
         * @brief push Pushes a value onto the VM's stack
         *
         * @param v The value to push
         *
         * @return True. If the stack is full the VM will have a panic attack.
         */
        bool push(const valued* v);

        /**
         * @brief push Pushes a number onto the stack.
         *
         * @param v the number to push
         *
         * @return True. If the stack is full the VM will have a panic attack.
         */
        bool push(const word_t v);

        /**
         * @brief pop Pops a number from the stack and returns it
         *
         * If the stack is empty the VM will throw a pani exception.
         *
         * @return The popped number.
         */
        word_t pop();

        /**
         * This method is called when the VM entered an exceptionally bad situation.
         * A dump of the memory is performed and the application will exit.
         */
        [[noreturn]] void panic() ;

        /**
         * @brief fetch Will return the next object from the virtual machine's memory.
         *
         * This gives an object in order for the opcode implementations to perform operation on them
         *
         * @return The next valued object which can be used to peform operations on them
         */
        valued* fetch();

        /**
         * @brief flag Returns the flag of the last operation.
         *
         * The flag is zero if the result of the last operation was zero, otherwise is not zero
         *
         * @return The VM's flag register.
         */
        word_t flag() const;

        /**
         * @brief set_flag Sets the flag to the given value
         *
         * @param v the new value of the flag
         */
        void set_flag(word_t v);

        /**
         * @brief jump Will execute a jump operation in the bytecode of the VM
         *
         * This method will not throw an exception if the VM's IP has left it's internal memory range
         *
         * @param v The new address to jump to
         *
         * @return True if the new address is inside the valid memory range of the VM, false if not
         */
        bool jump(word_t v);

        /**
         * @brief interrupt will perform the required interrupt
         *
         * After execution the state of the VM must be the one specified in the interrupts documentation.
         * Before the interrupt, the VM needs to set up its structures as per the interrupts requirements.
         *
         * @param i The interrupt number.
         * @return The value the interrupt handler returned
         */
        bool interrupt(word_t i);

        /**
         * @brief address_is_valid returns true if the given number can be mapped in the memory space of the VM
         * @param addr The address
         * @return True/False
         */
        bool address_is_valid(word_t addr);

    private:

        std::shared_ptr<vm_impl> impl;

    };

}


#endif

