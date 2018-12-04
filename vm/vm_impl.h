#ifndef VM_IMPL_H
#define VM_IMPL_H

#include <functional>
#include <map>
#include <hal.h>
#include <numeric_decl.h>
#include <registers.h>

#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <array>

namespace primal {

class vm;

struct vm_impl
{

    friend class vm;

    vm_impl();
    ~vm_impl();

    struct executor
    {
        std::function<bool(vm*)> runner;
    };

    bool run(const std::vector<uint8_t> &app, vm *v);

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


    void bindump(const char* title = nullptr, word_t start = -1, word_t end = -1, bool insert_addr = true);

    /**
     * @brief rsb is an access function for a given byte from a register
     */

    reg_subbyte* rsb(uint8_t ridx, uint8_t bidx);


    void set_mem(word_t address, word_t new_value);

    word_t get_mem(word_t address);

    void set_mem_byte(word_t address, uint8_t b);

    uint8_t get_mem_byte(word_t address);

    bool copy(word_t dest, word_t src, word_t cnt);

    // access the memory at the given location
    memaddress* mem(word_t address);
    memaddress_byte_ref* mem_byte(word_t address);
    immediate* imm(word_t v);

    type_destination fetch_type_dest();
    uint8_t fetch_register_index();
    word_t fetch_immediate();
    uint8_t fetch_byte();

    valued *fetch();


    word_t &ip();

    word_t ip() const;
    word_t flag() const;


    bool push(const valued* v);

    bool push(const word_t v);

    word_t pop();

private:

    static std::map<uint8_t, executor> opcode_runners;
    static std::map<word_t, executor> interrupts;

    reg m_r[VM_REG_COUNT];              // the registers of the machine
    word_t m_ip = 0;               // the instructions pointer
    std::unique_ptr<uint8_t[]> ms;      // the memory segment

    reg_subbyte t1 {&this->m_r[0], 0};
    std::array<immediate, 3> imv = { immediate{-1}, immediate{-1}, immediate{-1}} ;
    memaddress_byte_ref mb[2];
    memaddress ma[2];
    int mb_i = 0;
    int ma_i = 0;
    std::size_t mi_i = 0;
    reg& m_lbo; // the last operations' result. This is set to false if the result was 0
    word_t app_size = -1;
    word_t max_used_sp = 0;
    word_t stack_offset = 0;
    reg& sp;
};

}
#endif // VM_IMPL_H
