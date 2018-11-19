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

        // access function for a given sub reg byte
        reg_subbyte* rsb(uint8_t ridx, uint8_t bidx);

        // access the memory at the given location
        memaddress* mem(word_t address);
        memaddress_byte_ref* mem_byte(word_t address);
        immediate* imm(word_t v);

        type_destination fetch_type_dest();
        uint8_t fetch_register_index();
        word_t fetch_immediate();

        bool copy(word_t dest, word_t src, word_t cnt);

        bool push(const valued* v);
        bool push(const word_t v);

        word_t pop();



        [[noreturn]] void panic() ;

        valued* fetch();

        // the flag of the last operation
        word_t flag() const;
        word_t& flag();

        bool call(word_t v);

        // will perform the required interrupt
        // After execution the state of the VM must be the one specified in the interrupts documentation
        bool interrupt(word_t i);

        // returns true if the given number can be mapped in the memory space of the VM
        bool address_is_valid(word_t addr);

        void bindump(word_t start = -1, word_t end = -1, bool insert_addr = true);
        
    private:

        std::shared_ptr<vm_impl> impl;

    };

}


#endif
