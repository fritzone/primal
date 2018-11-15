#ifndef VM_H
#define VM_H

#include <hal.h>
#include <numeric_decl.h>
#include <registers.h>

#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <array>

namespace primal
{
    class vm final
    {

    public:

        vm();
        ~vm();

        static std::shared_ptr<vm> create();

        // will run the given application
        bool run(const std::vector<uint8_t>& app);

        // access function for the IP. You can use this to modify it though for faster access
        numeric_t& ip()      {return m_ip;}
        numeric_t ip() const {return m_ip;}

        // will set the memory at the given addres to the new value
        void set_mem(numeric_t address, numeric_t new_value);
        numeric_t get_mem(numeric_t address);

        void set_mem_byte(numeric_t address, uint8_t b);
        uint8_t get_mem_byte(numeric_t address);

        // access function for the given register. You can use this to modify it though for faster access
        reg& r(uint8_t i)             { return m_r[i];}
        const reg& r(uint8_t i) const { return m_r[i];}

        // access function for a given sub reg byte
        reg_subbyte* rsb(uint8_t ridx, uint8_t bidx);

        // access the memory at the given location
        memaddress* mem(numeric_t address);
        memaddress_byte_ref* mem_byte(numeric_t address);
        immediate* imm(numeric_t v);

        type_destination fetch_type_dest();
        uint8_t fetch_register_index();
        numeric_t fetch_immediate();

        bool copy(numeric_t dest, numeric_t src, numeric_t cnt);

        bool push(const valued* v);
        bool push(const numeric_t v);

        numeric_t pop();

        template<class OPC, class EXECUTOR>
        static void register_opcode(OPC&& o, EXECUTOR&& ex)
        {
            auto f = [&](vm* machina) -> bool {return ex(machina);};
            executor t;
            t.runner = std::function<bool(vm*)>(f);
            opcode_runners[o.bin()] = t;
        };

        template<class EXECUTOR>
        static void register_interrupt(uint8_t intrn, EXECUTOR&& ex)
        {
            auto f = [&](vm* machina) -> bool {return ex(machina);};
            executor t;
            t.runner = std::function<bool(vm*)>(f);
            interrupts[intrn] = t;
        }

        [[noreturn]] void panic() ;

        valued* fetch();

        // the flag of the last operation
        numeric_t flag() const {return m_lbo.value();}
        numeric_t& flag() {return m_lbo.value();}

        bool call(numeric_t v);

        // will perform the required interrupt
        // After execution the state of the VM must be the one specified in the interrupts documentation
        bool interrupt(uint8_t i);

        // returns true if the given number can be mapped in the memory space of the VM
        bool address_is_valid(numeric_t addr);

        void bindump(numeric_t start = -1, numeric_t end = -1, bool insert_addr = true);
        
    private:

        struct executor
        {
            std::function<bool(vm*)> runner;
        };

        static std::map<uint8_t, executor> opcode_runners;
        static std::map<uint8_t, executor> interrupts;

        reg m_r[VM_REG_COUNT];              // the registers of the machine
        numeric_t m_ip = 0;               // the instructions pointer
        std::unique_ptr<uint8_t[]> ms;      // the memory segment

        reg_subbyte t1 {&this->r(0), 0};
        std::array<immediate, 3> imv = { immediate{-1}, immediate{-1}, immediate{-1}} ;
        memaddress_byte_ref mb[2];
        memaddress ma[2];
        int mb_i = 0;
        int ma_i = 0;
        std::size_t mi_i = 0;
        reg& m_lbo; // the last operations' result. This is set to false if the result was 0
        numeric_t app_size = -1;
        numeric_t max_used_sp = 0;
        numeric_t stack_offset = 0;
        reg& sp;
    };

}


#endif
