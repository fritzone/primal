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

    vm_impl();
    ~vm_impl();

    struct executor
    {
        std::function<bool(vm*)> runner;
    };

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


    static std::map<uint8_t, executor> opcode_runners;
    static std::map<uint8_t, executor> interrupts;

    reg m_r[VM_REG_COUNT];              // the registers of the machine
    numeric_t m_ip = 0;               // the instructions pointer
    std::unique_ptr<uint8_t[]> ms;      // the memory segment

    reg_subbyte t1 {&this->m_r[0], 0};
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
#endif // VM_IMPL_H
